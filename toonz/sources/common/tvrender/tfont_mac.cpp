

#ifdef __LP64__

#include <vector>
#include <iostream>
#include <string>
#include "tmathutil.h"
#include "tdebugmessage.h"
#include "tfont.h"
#include "tstroke.h"
#include "tcurves.h"
#include "tconvert.h"
#include "tvectorimage.h"

//#include <CoreText/CoreText.h>
#include <QFont>
#include <QfontDatabase>

using namespace std;

struct TFont::Impl {
	QFont m_qfont;
	bool m_hasKerning;
	int m_hasVertical;
	Fixed m_size;
	int m_ascender;
	int m_descender;

	Impl(QFont* qfont);
	~Impl();
};

//-----------------------------------------------------------------------------

TFont::TFont(QFont* qfont)
{
	m_pimpl = new Impl(qfont);
}

//-----------------------------------------------------------------------------

TFont::~TFont()
{
	delete m_pimpl;
}

//-----------------------------------------------------------------------------

TFont::Impl::Impl(QFont* qfont)
	: m_size(0), m_qfont(*qfont)
{
	OSStatus status;

	long response;
	ByteCount sizes[2];
}

//-----------------------------------------------------------------------------

TFont::Impl::~Impl()
{
}

//-----------------------------------------------------------------------------

TPoint TFont::drawChar(TVectorImageP &image, wchar_t charcode, wchar_t nextCharCode) const
{
	OSStatus status;

	UniChar subString[2];
	subString[0] = charcode;
	subString[1] = 0 /*nextCharCode*/;
	UniCharCount length = sizeof(subString) / sizeof(UniChar);

	image->transform(TScale(1, -1));

	image->group(0, image->getStrokeCount());

	return getDistance(charcode, nextCharCode);
}

//-----------------------------------------------------------------------------
namespace
{

void appDrawChar(TRasterGR8P &outImage, TFont::Impl *pimpl, wchar_t charcode)
{
	OSStatus status;
	UniChar subString[2];
	subString[0] = charcode;
	subString[1] = 0;
	UniCharCount length = sizeof(subString) / sizeof(UniChar);

	TPixelGR8 bgp;
	bgp.value = 255;
	outImage->fill(bgp);
	void *data = outImage->getRawData();

	CGColorSpaceRef grayColorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericGray);
}
}
//-----------------------------------------------------------------------------

TPoint TFont::drawChar(TRasterGR8P &outImage, TPoint &unused, wchar_t charcode, wchar_t nextCharCode) const
{
	appDrawChar(outImage, m_pimpl, charcode);
	outImage->yMirror();
	return getDistance(charcode, nextCharCode);
}

//-----------------------------------------------------------------------------

TPoint TFont::drawChar(TRasterCM32P &outImage, TPoint &unused, int inkId, wchar_t charcode, wchar_t nextCharCode) const
{
	TRasterGR8P grayAppImage;
	appDrawChar(grayAppImage, m_pimpl, charcode);

	int lx = grayAppImage->getLx();
	int ly = grayAppImage->getLy();

	outImage = TRasterCM32P(lx, ly);

	assert(TPixelCM32::getMaxTone() == 255);
	TPixelCM32 bgColor(0, 0, TPixelCM32::getMaxTone());
	grayAppImage->lock();
	outImage->lock();
	int ty = 0;
	for (int gy = ly - 1; gy >= 0; --gy, ++ty) {
		TPixelGR8 *srcPix = grayAppImage->pixels(gy);
		TPixelCM32 *tarPix = outImage->pixels(ty);
		for (int x = 0; x < lx; ++x) {
			int tone = srcPix->value;

			if (tone == 255)
				*tarPix = bgColor;
			else
				*tarPix = TPixelCM32(inkId, 0, tone);

			++srcPix;
			++tarPix;
		}
	}
	grayAppImage->unlock();
	outImage->unlock();

	return getDistance(charcode, nextCharCode);
}

//-----------------------------------------------------------------------------

TPoint TFont::getDistance(wchar_t firstChar, wchar_t secondChar) const
{
	OSStatus status;
	UniChar subString[2];
	subString[0] = firstChar;
	subString[1] = secondChar;
	UniCharCount length = sizeof(subString) / sizeof(UniChar);
	ItemCount numGlyphs;

	assert(numGlyphs >= 2);

	return TPoint(1, 0);
}

//-----------------------------------------------------------------------------

int TFont::getMaxHeight() const
{
	return m_pimpl->m_ascender - m_pimpl->m_descender;
}

//-----------------------------------------------------------------------------

int TFont::getMaxWidth() const
{
	assert(!"not implemented yet");
	return 100;
}
//-----------------------------------------------------------------------------

int TFont::getLineAscender() const
{
	return m_pimpl->m_ascender;
}

//-----------------------------------------------------------------------------

int TFont::getLineDescender() const
{
	return m_pimpl->m_descender;
}

//-----------------------------------------------------------------------------

bool TFont::hasKerning() const
{
	return true;
}

//-----------------------------------------------------------------------------

bool TFont::hasVertical() const
{
	return false;
}

//-----------------------------------------------------------------------------

#include <string>
#include <map>

struct TFontManager::Impl {
	QFontDatabase m_database;
	bool m_loaded;
//	ATSUFontID m_currentAtsuFontId;
	TFont *m_currentFont;
	QFont *m_currentQFont;
//	QString m_currentFamily;
	wstring m_currentTypeface;
	int m_size;

	Impl()
		: m_currentFont(0), m_currentQFont(), m_loaded(false), m_size(70), m_database()
	{
	}

//	bool setFontName(ATSUFontID fontId, int platform, int script, int lang);
//	bool addFont(ATSUFontID);
	void loadFontNames();
	bool setFont(std::wstring family);
};

using namespace std;

//bool TFontManager::Impl::setFontName(ATSUFontID fontId, int platform, int script, int lang)
//{
//	ByteCount oActualNameLength;
//	ItemCount oFontCount;
//	OSStatus status;
//
//	char *buffer = 0;
//	char *buffer2 = 0;
//
//	return false;
//}

//bool TFontManager::Impl::addFont(ATSUFontID fontId)
//{
//	int platform, script, lang;
//
//	// per ottimizzare, ciclo solo sui valori
//	// piu' comuni
//	for (lang = -1; lang <= 0; lang++)
//		for (platform = -1; platform <= 1; platform++)
//			for (script = -1; script <= 0; script++)
//				if (setFontName(fontId, platform, script, lang))
//					return true;
//
//	//poi li provo tutti
//	for (lang = -1; lang <= 139; lang++)
//		for (script = -1; script <= 32; script++)
//			for (platform = -1; platform <= 4; platform++) {
//				// escludo quelli nel tri-ciclo for precedente.
//				// Purtoppo si deve fare cosi:
//				// non si puo' fare partendo con indici piu' alti nei cicli for!
//				if (-1 <= lang && lang <= 0 &&
//					-1 <= script && script <= 0 &&
//					-1 <= platform && platform <= 1)
//					continue;
//
//				if (setFontName(fontId, platform, script, lang))
//					return true;
//			}
//
//	return false;
//}

void TFontManager::Impl::loadFontNames()
{
	return;
}

bool TFontManager::Impl::setFont(std::wstring family)
{
	for(const QString eachFamily: m_database.families()){
		auto famstr = eachFamily.toStdWString();
		if(eachFamily.toStdWString() == family){
//			m_currentFamily = eachFamily;
			m_currentQFont = new QFont(eachFamily);
			m_currentFont = new TFont(m_currentQFont);
//			for(const QString eachStyle: m_database.styles(eachFamily)){
//				break;
//			}
			break;
		}
	}
	return true;
}

//---------------------------------------------------------

TFontManager::TFontManager()
{
	m_pimpl = new TFontManager::Impl();
}

//---------------------------------------------------------

TFontManager::~TFontManager()
{
	delete m_pimpl;
}

//---------------------------------------------------------

TFontManager *TFontManager::instance()
{
	static TFontManager theManager;
	return &theManager;
}

//---------------------------------------------------------

void TFontManager::loadFontNames()
{
	m_pimpl->loadFontNames();
}

//---------------------------------------------------------

void TFontManager::setFamily(const wstring family)
{
	m_pimpl->setFont(family);
//	if (changed) {
//		delete m_pimpl->m_currentFont;
//		m_pimpl->m_currentFont = new TFont(m_pimpl->m_currentAtsuFontId, m_pimpl->m_size);
//	}
}

//---------------------------------------------------------

void TFontManager::setTypeface(const wstring typeface)
{
	return;
}

//---------------------------------------------------------

void TFontManager::setSize(int size)
{
}

//---------------------------------------------------------

wstring TFontManager::getCurrentFamily() const
{
	return m_pimpl->m_currentQFont->family().toStdWString();
}

//---------------------------------------------------------

wstring TFontManager::getCurrentTypeface() const
{
	return m_pimpl->m_currentTypeface;
}

//---------------------------------------------------------

TFont *TFontManager::getCurrentFont()
{
	if (m_pimpl->m_currentFont)
		return m_pimpl->m_currentFont;

	if (!m_pimpl->m_currentFont)
		loadFontNames();

//	assert(!m_pimpl->m_families.empty());
//	setFamily(toWideString(m_pimpl->m_families.begin()->first));

	return m_pimpl->m_currentFont;
}

//---------------------------------------------------------

void TFontManager::getAllFamilies(vector<wstring> &families) const
{
	families.clear();
	for(const QString eachFamily: m_pimpl->m_database.families()){
		families.push_back(eachFamily.toStdWString());
	}
}

//---------------------------------------------------------

void TFontManager::getAllTypefaces(vector<wstring> &typefaces) const
{
	typefaces.clear();
	for(const QString eachStyle: m_pimpl->m_database.styles(m_pimpl->m_currentQFont->family())){
		typefaces.push_back(eachStyle.toStdWString());
	}
}

//---------------------------------------------------------

void TFontManager::setVertical(bool vertical) {}

//---------------------------------------------------------

#endif
