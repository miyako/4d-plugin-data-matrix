/*
 Synthetic-boundary ownership harness for the DMTX plugin.

 The real plugin source (4DPlugin-DMTX.cpp), the real bundled SDK (4DPluginAPI.c and Classes)
 and the bundled libdmtx/libpng/libjpeg are linked unchanged; only the 4D engine behind
 gCall4D is replaced by the fake below, which tracks every picture, string and handle it hands out.

 Engine model (assumptions, not verified against a running 4D):
 - A PA_Variable owns its picture/string: PA_ClearVariable disposes it.
 - A command that writes a picture into a variable (CREATE THUMBNAIL destination, CONVERT PICTURE
   in place) disposes the picture the variable held before and stores a new picture owned by the
   variable, like an assignment in 4D code. --convert-noop models a CONVERT PICTURE that leaves
   the variable unchanged (e.g. conversion failure).
 - Pictures are reference counted: PA_DuplicatePicture(p, 1) retains, PA_DisposePicture releases.
 - PA_GetPictureData fills the handle and returns an empty PA_Unistring (fString == NULL).
 - PA_SetCollectionElement deep-copies the value.
 - PA_IsCompiled(0) follows --compiled.
*/

#include "4DPluginAPI.h"
#include "PrivateTypes.h"
#include "EntryPoints.h"

#import <Foundation/Foundation.h>
#import <ImageIO/ImageIO.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

void PluginMain(PA_long32 selector, PA_PluginParameters params);

namespace engine {

static long livePictures = 0, liveStrings = 0, liveHandles = 0;
static bool compiled = false;
static bool convertReplaces = true;

struct Picture {
    unsigned magic;
    int refs;
    std::vector<uint8_t> bytes;
};

struct Handle {
    std::vector<char> bytes;
};

struct Collection {
    std::vector<std::u16string> strings;
};

static Picture *newPicture(const void *buf, size_t len)
{
    Picture *p = new Picture;
    p->magic = 'PICT';
    p->refs = 1;
    if(buf && len) p->bytes.assign((const uint8_t *)buf, (const uint8_t *)buf + len);
    livePictures++;
    return p;
}

static Picture *asPicture(PA_Picture pic)
{
    Picture *p = (Picture *)pic;
    if(p && (p->magic != 'PICT' || p->refs <= 0)) {
        fprintf(stderr, "FAKE ENGINE: invalid picture %p\n", pic);
        abort();
    }
    return p;
}

static void releasePicture(PA_Picture pic)
{
    Picture *p = (Picture *)pic;
    if(!p) return;
    /* reading refs of an already-deleted picture is a heap-use-after-free under ASan */
    if(--p->refs == 0) {
        p->magic = 0;
        delete p;
        livePictures--;
    }
}

static PA_Unistring newString(const PA_Unichar *s)
{
    PA_Unistring u = {0, NULL, 0, 0};
    size_t len = 0;
    if(s) while(s[len]) len++;
    u.fString = new PA_Unichar[len + 1];
    if(len) memcpy(u.fString, s, len * sizeof(PA_Unichar));
    u.fString[len] = 0;
    u.fLength = (PA_long32)len;
    liveStrings++;
    return u;
}

static void disposeString(PA_Unistring &u)
{
    if(u.fString) {
        delete [] u.fString;
        liveStrings--;
    }
    u.fString = NULL;
    u.fLength = 0;
}

static void clearVariable(PA_Variable *v)
{
    switch(v->fType) {
        case eVK_Picture: releasePicture(v->uValue.fPicture); break;
        case eVK_Unistring: disposeString(v->uValue.fString); break;
        default: break;
    }
    v->fType = eVK_Undefined;
}

/* a command writes a new picture into a variable: the old one is released, like a 4D assignment */
static void assignPicture(PA_Variable *v, Picture *p)
{
    clearVariable(v);
    v->fType = eVK_Picture;
    v->uValue.fPicture = p;
}

static void executeCommand(EngineBlock *eb)
{
    PA_Variable **vars = (PA_Variable **)eb->fPtr1;
    PA_Variable *returned = (PA_Variable *)eb->fPtr2;
    switch(eb->fParam1) {
        case 1472: /* New collection */
            returned->fType = eVK_Collection;
            returned->uValue.fCollection = new Collection;
            break;
        case 679: { /* CREATE THUMBNAIL(source; dest; w; h; mode) */
            Picture *src = asPicture(vars[0]->fType == eVK_Picture ? vars[0]->uValue.fPicture : NULL);
            assignPicture(vars[1], src ? newPicture(src->bytes.data(), src->bytes.size()) : newPicture(NULL, 0));
            break;
        }
        case 1002: { /* CONVERT PICTURE(picture; codec) */
            if(!convertReplaces) break;
            Picture *src = asPicture(vars[0]->fType == eVK_Picture ? vars[0]->uValue.fPicture : NULL);
            Picture *converted = src ? newPicture(src->bytes.data(), src->bytes.size()) : newPicture(NULL, 0);
            assignPicture(vars[0], converted);
            break;
        }
        case 356: { /* Picture size */
            Picture *src = asPicture(vars[0]->fType == eVK_Picture ? vars[0]->uValue.fPicture : NULL);
            returned->fType = eVK_Longint;
            returned->uValue.fLongint = src ? (PA_long32)src->bytes.size() : 0;
            break;
        }
        default:
            fprintf(stderr, "FAKE ENGINE: unexpected command %ld\n", (long)eb->fParam1);
            abort();
    }
}

static void handleManager(EngineBlock *eb)
{
    Handle *h = (Handle *)eb->fHandle;
    switch(eb->fParam1) {
        case 1: h = new Handle; h->bytes.resize((size_t)eb->fParam2); liveHandles++; eb->fHandle = (PA_Handle)h; break;
        case 2: delete h; liveHandles--; break;
        case 3: h->bytes.resize((size_t)eb->fParam2); break;
        case 4: eb->fParam2 = (sLONG_PTR)h->bytes.size(); break;
        case 5: eb->fParam3 = (sLONG_PTR)h->bytes.data(); break;
        default: break;
    }
}

static void call4D(short selector, EngineBlock *eb)
{
    switch(selector) {
        case EX_HANDLE_MANAGER: handleManager(eb); break;
        case EX_CREATE_UNISTRING: eb->fUniString1 = newString((const PA_Unichar *)eb->fHandle); break;
        case EX_DISPOSE_UNISTRING: disposeString(eb->fUniString1); break;
        case EX_SET_UNISTRING: {
            PA_Unistring u = newString((const PA_Unichar *)eb->fHandle);
            disposeString(eb->fUniString1);
            eb->fUniString1 = u;
            break;
        }
        case EX_CREATE_PICTURE: eb->fPicture = newPicture(eb->fPtr1, (size_t)eb->fParam1); break;
        case EX_DISPOSE_PICTURE: releasePicture(eb->fPicture); break;
        case EX_DUPLICATE_PICTURE: {
            Picture *p = asPicture(eb->fPicture);
            if(!p) break;
            if(eb->fParam1 == 0) { p->refs++; } /* retainOnly */
            else eb->fPicture = newPicture(p->bytes.data(), p->bytes.size());
            break;
        }
        case EX_GET_PICTURE_DATA: {
            Picture *p = asPicture(eb->fPicture);
            eb->fUniString1.fLength = 0; eb->fUniString1.fString = NULL;
            if(!p || eb->fParam1 != 1 || p->bytes.empty()) { eb->fError = -1; break; }
            if(eb->fHandle) ((Handle *)eb->fHandle)->bytes.assign(p->bytes.begin(), p->bytes.end());
            break;
        }
        case EX_CREATE_NATIVE_PICTURE_FOR_SCREEN: {
            Picture *p = asPicture(eb->fPicture);
            CGImageRef image = NULL;
            if(p && !p->bytes.empty()) {
                CFDataRef data = CFDataCreate(NULL, p->bytes.data(), (CFIndex)p->bytes.size());
                CGImageSourceRef source = CGImageSourceCreateWithData(data, NULL);
                if(source) { image = CGImageSourceCreateImageAtIndex(source, 0, NULL); CFRelease(source); }
                CFRelease(data);
            }
            eb->fHandle = (PA_Handle)image;
            break;
        }
        case EX_EXECUTE_COMMAND_BY_ID: executeCommand(eb); break;
        case EX_CLEAR_VARIABLE: clearVariable((PA_Variable *)eb->fPtr1); break;
        case EX_GET_INFORMATION: eb->fParam1 = compiled ? 0x08000000 : 0; break;
        case EX_YIELD_ABSOLUTE: break;
        case EX_SET_OBJ_VALUE: {
            Collection *c = (Collection *)eb->fPtr1;
            PA_Variable *v = (PA_Variable *)eb->fPtr2;
            if(v->fType != eVK_Unistring) { eb->fError = -1; break; }
            std::u16string s((const char16_t *)v->uValue.fString.fString, (size_t)v->uValue.fString.fLength);
            if((size_t)eb->fLongint >= c->strings.size()) c->strings.resize((size_t)eb->fLongint + 1);
            c->strings[(size_t)eb->fLongint] = s; /* deep copy */
            break;
        }
        case EX_GET_OBJ_VALUE: {
            Collection *c = (Collection *)eb->fPtr1;
            PA_Variable *v = (PA_Variable *)eb->fPtr2;
            v->fType = eVK_Longint;
            v->uValue.fLongint = (PA_long32)c->strings.size();
            break;
        }
        default:
            fprintf(stderr, "FAKE ENGINE: unexpected selector %d\n", selector);
            abort();
    }
}

} // namespace engine

static void initPlugin()
{
    PackInitBlock init;
    memset(&init, 0, sizeof(init));
    init.fCall4Dex = engine::call4D;
    FourDPackex(kInitPlugin, &init, NULL, NULL);
}

static std::u16string toU16(const char *s)
{
    std::u16string u;
    while(*s) u.push_back((char16_t)*s++);
    return u;
}

static const char *kText = "HELLO 4D DMTX";

struct Encoded {
    PA_Picture picture;
    PA_Unistring data;
};

static Encoded encode(PA_long32 format)
{
    std::u16string text = toU16(kText);
    PA_Unistring in = engine::newString((const PA_Unichar *)text.c_str());
    PA_long32 outputFormat = format, scheme = 0, moduleSize = 5, symbolSize = -2, margin = 2, dpi = 72;
    Encoded e = {NULL, {0, NULL, 0, 0}};
    void *params[8] = {&in, &outputFormat, &scheme, &moduleSize, &symbolSize, &margin, &dpi, &e.data};
    FourDPackex(1, params, NULL, &e.picture);
    engine::disposeString(in);
    return e;
}

static void report(const char *name, bool ok, long pictures, long strings, long handles, const std::string &detail)
{
    printf("%s %s pictures=%+ld strings=%+ld handles=%+ld %s\n", ok ? "PASS" : "FAIL", name, pictures, strings, handles, detail.c_str());
}

/* encode a symbol, then read it back through DMTX Read image (selector 2) or DMTX Read images (selector 3) */
static int readScenario(const char *name, PA_long32 selector)
{
    Encoded e = encode(2);
    engine::disposeString(e.data);
    if(!e.picture) { report(name, false, 0, 0, 0, "encode failed"); return 1; }

    long p0 = engine::livePictures, s0 = engine::liveStrings, h0 = engine::liveHandles;
    std::string decoded;

    if(selector == 2) {
        PA_Unistring result = {0, NULL, 0, 0};
        PA_Unistring *pResult = &result;
        void *params[1] = {&e.picture};
        FourDPackex(selector, params, NULL, &pResult);
        for(PA_long32 i = 0; i < result.fLength; i++) decoded.push_back((char)result.fString[i]);
        engine::disposeString(result);
    } else {
        PA_CollectionRef result = NULL;
        void *params[1] = {&e.picture};
        FourDPackex(selector, params, NULL, &result);
        engine::Collection *c = (engine::Collection *)result;
        if(c && c->strings.size() == 1) for(char16_t ch : c->strings[0]) decoded.push_back((char)ch);
        else decoded = c ? "<" + std::to_string(c->strings.size()) + " elements>" : "<no collection>";
        delete c;
    }

    long dp = engine::livePictures - p0, ds = engine::liveStrings - s0, dh = engine::liveHandles - h0;
    PA_DisposePicture(e.picture);
    bool ok = (dp == 0) && (ds == 0) && (dh == 0) && (decoded == kText)
        && engine::livePictures == 0 && engine::liveStrings == 0 && engine::liveHandles == 0;
    report(name, ok, dp, ds, dh, "decoded=\"" + decoded + "\"");
    return ok ? 0 : 1;
}

static int encodeScenario(const char *name, PA_long32 format)
{
    Encoded e = encode(format);
    bool hasPicture = e.picture && !engine::asPicture(e.picture)->bytes.empty();
    bool hasData = e.data.fLength > 0;
    long pictures = engine::livePictures;
    PA_DisposePicture(e.picture);
    engine::disposeString(e.data);
    bool ok = hasPicture && hasData && pictures == 1
        && engine::livePictures == 0 && engine::liveStrings == 0 && engine::liveHandles == 0;
    report(name, ok, engine::livePictures, engine::liveStrings, engine::liveHandles,
           std::string("result=") + (hasPicture ? "picture" : "none") + " data=" + (hasData ? "set" : "empty"));
    return ok ? 0 : 1;
}

int main(int argc, const char *argv[])
{
    @autoreleasepool {
        if(argc < 2) { fprintf(stderr, "usage: %s scenario [--compiled] [--convert-noop]\n", argv[0]); return 2; }
        std::string scenario = argv[1];
        for(int i = 2; i < argc; i++) {
            if(!strcmp(argv[i], "--compiled")) engine::compiled = true;
            if(!strcmp(argv[i], "--convert-noop")) engine::convertReplaces = false;
        }
        initPlugin();

        if(scenario == "read_image") return readScenario(argv[1], 2);
        if(scenario == "read_images") return readScenario(argv[1], 3);
        if(scenario == "ctl_encode_png") return encodeScenario(argv[1], 2);
        if(scenario == "ctl_encode_svg") return encodeScenario(argv[1], 1);
        if(scenario == "ctl_encode_jpg") return encodeScenario(argv[1], 0);
        if(scenario == "ctl_detector_leak") {
            /* positive control: an undisposed picture must be reported */
            PA_Picture p = PA_CreatePicture((void *)"x", 1);
            (void)p;
            bool ok = engine::livePictures == 0;
            report(argv[1], ok, engine::livePictures, engine::liveStrings, engine::liveHandles, "");
            return ok ? 0 : 1;
        }
        if(scenario == "ctl_detector_double_dispose") {
            /* positive control: disposing a picture twice must abort under ASan */
            PA_Picture p = PA_CreatePicture((void *)"x", 1);
            PA_DisposePicture(p);
            PA_DisposePicture(p);
            report(argv[1], true, engine::livePictures, engine::liveStrings, engine::liveHandles, "double dispose not detected");
            return 0;
        }
        fprintf(stderr, "unknown scenario %s\n", argv[1]);
        return 2;
    }
}
