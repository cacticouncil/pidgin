/*
The contents of this file are subject to the Mozilla Public License
Version 1.1 (the "License"); you may not use this file except in
compliance with the License. You may obtain a copy of the License at
http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS"
basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
License for the specific language governing rights and limitations
under the License.

The Original Code is expat.

The Initial Developer of the Original Code is James Clark.
Portions created by James Clark are Copyright (C) 1998, 1999
James Clark. All Rights Reserved.

Contributor(s):

*/

const ENCODING *NS(XmlGetUtf8InternalEncoding)()
{
    return &ns(internal_utf8_encoding).enc;
}

const ENCODING *NS(XmlGetUtf16InternalEncoding)()
{
#if XML_BYTE_ORDER == 12
    return &ns(internal_little2_encoding).enc;
#elif XML_BYTE_ORDER == 21
return &ns(internal_big2_encoding).enc;
#else
const short n = 1;
    return *(const char *)&n ? &ns(internal_little2_encoding).enc : &ns(internal_big2_encoding).enc;
#endif
}

static
const ENCODING *NS(encodings)[] = {
    &ns(latin1_encoding).enc,
    &ns(ascii_encoding).enc,
    &ns(utf8_encoding).enc,
    &ns(big2_encoding).enc,
    &ns(big2_encoding).enc,
    &ns(little2_encoding).enc,
    &ns(utf8_encoding).enc /* NO_ENC */
};

static
int NS(initScanProlog)(const ENCODING *enc, const char *ptr, const char *end,
                       const char **nextTokPtr)
{
    return initScan(NS(encodings), (const INIT_ENCODING *)enc, XML_PROLOG_STATE, ptr, end, nextTokPtr);
}

static
int NS(initScanContent)(const ENCODING *enc, const char *ptr, const char *end,
                        const char **nextTokPtr)
{
    return initScan(NS(encodings), (const INIT_ENCODING *)enc, XML_CONTENT_STATE, ptr, end, nextTokPtr);
}

int NS(XmlInitEncoding)(INIT_ENCODING *p, const ENCODING **encPtr, const char *name)
{
    int i = getEncodingIndex(name);
    if (i == UNKNOWN_ENC)
        return 0;
    INIT_ENC_INDEX(p) = (char)i;
    p->initEnc.scanners[XML_PROLOG_STATE] = NS(initScanProlog);
    p->initEnc.scanners[XML_CONTENT_STATE] = NS(initScanContent);
    p->initEnc.updatePosition = initUpdatePosition;
    p->encPtr = encPtr;
    *encPtr = &(p->initEnc);
    return 1;
}

static
const ENCODING *NS(findEncoding)(const ENCODING *enc, const char *ptr, const char *end)
{
#define ENCODING_MAX 128
    char buf[ENCODING_MAX];
    char *p = buf;
    int i;
    XmlUtf8Convert(enc, &ptr, end, &p, p + ENCODING_MAX - 1);
    if (ptr != end)
        return 0;
    *p = 0;
    if (streqci(buf, "UTF-16") && enc->minBytesPerChar == 2)
        return enc;
    i = getEncodingIndex(buf);
    if (i == UNKNOWN_ENC)
        return 0;
    return NS(encodings)[i];
}

int NS(XmlParseXmlDecl)(int isGeneralTextEntity,
                        const ENCODING *enc,
                        const char *ptr,
                        const char *end,
                        const char **badPtr,
                        const char **versionPtr,
                        const char **encodingName,
                        const ENCODING **encoding,
                        int *standalone)
{
    return doParseXmlDecl(NS(findEncoding),
                          isGeneralTextEntity,
                          enc,
                          ptr,
                          end,
                          badPtr,
                          versionPtr,
                          encodingName,
                          encoding,
                          standalone);
}
