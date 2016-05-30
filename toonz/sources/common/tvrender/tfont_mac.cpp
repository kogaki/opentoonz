

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
#include "trop.h"

//#include <CoreText/CoreText.h>
#include <QFont>
#include <QfontDatabase>
#include <QPainter>
#include <QLabel>
#include <QWidget>


using namespace std;

QImage rasterToQImage(const TRasterP &ras, bool premultiplied = true, bool mirrored = true)
{
	if (TRaster32P ras32 = ras) {
		QImage image(ras->getRawData(), ras->getLx(), ras->getLy(),
					 premultiplied ? QImage::Format_ARGB32_Premultiplied : QImage::Format_ARGB32);
		if (mirrored)
			return image.mirrored();
		return image;
	} else if (TRasterGR8P ras8 = ras) {
		QImage image(ras->getRawData(), ras->getLx(), ras->getLy(), ras->getWrap(), QImage::Format_Indexed8);
		static QVector<QRgb> colorTable;
		if (colorTable.size() == 0) {
			int i;
			for (i = 0; i < 256; i++)
				colorTable.append(QColor(i, i, i).rgb());
		}
		image.setColorTable(colorTable);
		if (mirrored)
			return image.mirrored();
		return image;
	}
	return QImage();
}

//TRaster32P rasterFromQImage(QImage image, bool premultiply = true, bool mirror = true) //no need of const& - Qt uses implicit sharing...
//{
//	QImage copyImage = mirror ? image.mirrored() : image;
//	TRaster32P ras(image.width(), image.height(), image.width(), (TPixelRGBM32 *)copyImage.bits(), false);
//	if (premultiply)
//		TRop::premultiply(ras);
//	return ras->clone();
//}
TRasterP rasterFromQImage(const QImage &image) //no need of const& - Qt uses implicit sharing...
{
	QImage::Format format = image.format();
	if (format == QImage::Format_ARGB32 || format == QImage::Format_ARGB32_Premultiplied) {
		TRaster32P rimage(image.width(), image.height(), image.width(), (TPixelRGBM32 *) image.bits(), false);
		rimage->addRef();
		return rimage;
	}
	if (format == QImage::Format_Indexed8){
//		return TRasterGR8P(image.width(), image.height());
		return TRasterGR8P(image.width(), image.height(), image.bytesPerLine(), (TPixelGR8 *)image.bits(), false);
	}
	return TRasterP();
}


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


TStroke convertPointsToStroke(std::vector<TThickPoint> points){
	TStroke *stroke = new TStroke();
	// to close loop
	TThickPoint lastPoint = points.back();
	TThickPoint firstPoint = points.front();
	if(!isAlmostZero(lastPoint.x - firstPoint.x) || !isAlmostZero(lastPoint.y - firstPoint.y)){
		points.push_back((lastPoint + firstPoint) * 0.5);
		points.push_back(firstPoint);
	}

	// TODO: maybe bug
	if(points.size() % 2 == 0){
		points.push_back(firstPoint);
	}


	stroke->reshape(&(points[0]), points.size());
	stroke->setSelfLoop(true);
	return *stroke;
}

std::vector<TStroke> convertQPainterPathToStrokes(const QPainterPath pathes){
	std::vector<TStroke> strokes;
	std::vector<TThickPoint> points;
	for(int i=0; i<pathes.elementCount(); i++){
		auto element = pathes.elementAt(i);
		TThickPoint point = TThickPoint(element.x, element.y, 0);
		if(element.isMoveTo()){ // stroke start
			if(!points.empty()){
				strokes.push_back(convertPointsToStroke(points));
			}
			points.clear();
		}
		points.push_back(point);
	}
	if(!points.empty()){
		strokes.push_back(convertPointsToStroke(points));
	}
	return strokes;
}

TPoint TFont::drawChar(TVectorImageP &image, wchar_t charcode, wchar_t nextCharCode) const
{
    QPainterPath pathes;
    wchar_t wchararray[1] = {charcode};
	auto qstr = QString::fromWCharArray(wchararray);
	pathes.addText(10, 30, m_pimpl->m_qfont, qstr);

	QImage qimage(70, 70, QImage::Format_ARGB32);
	QPainter painter(&qimage);
	painter.fillRect(qimage.rect(), Qt::white);
	painter.drawPath(pathes);
	painter.end();
	QLabel* lbl = new QLabel();
	lbl->setPixmap(QPixmap::fromImage(qimage));
	lbl->show();



//	std::vector<TStroke> strokes;
//	std::vector<TThickPoint> points;
//	for(int i=0; i<pathes.elementCount(); i++){
//		auto element = pathes.elementAt(i);
//		TThickPoint point = TThickPoint(element.x, element.y, 0);
//		if(element.isMoveTo()){ // stroke start
//			if(!points.empty()){
//				strokes.push_back(convertPointsToStroke(points));
//			}
//			points.clear();
//		}
//		points.push_back(point);
//	}
//	if(!points.empty()){
//		strokes.push_back(convertPointsToStroke(points));
//	}
//
//
	std::vector<TThickPoint> points;
	points.clear();
	for(int i=0; i<pathes.elementCount(); i++){
		auto element = pathes.elementAt(i);
		TThickPoint startPoint = TThickPoint(element.x, element.y, 0);
		points.push_back(startPoint);
	}

	// to close loop
	TThickPoint lastPoint = points.back();
	TThickPoint firstPoint = points.front();
	if(!isAlmostZero(lastPoint.x - firstPoint.x) || !isAlmostZero(lastPoint.y - firstPoint.y)){
		points.push_back((lastPoint + firstPoint) * 0.5);
		points.push_back(firstPoint);
	}

	TStroke *stroke = new TStroke();
	stroke->reshape(&(points[0]), points.size());
	stroke->setSelfLoop(true);
//
//	for(auto stroke: convertQPainterPathToStrokes(pathes)){
//		image->addStroke(&stroke);
//		break;
//	}

	image->addStroke(stroke);
	image->group(0, image->getStrokeCount());

	image->transform(TScale(1, -1));
	image->group(0, image->getStrokeCount());


	UniChar subString[2];
	subString[0] = charcode;
	subString[1] = 0 /*nextCharCode*/;
	UniCharCount length = sizeof(subString) / sizeof(UniChar);


	return getDistance(charcode, nextCharCode);
}

//-----------------------------------------------------------------------------
namespace
{


// TODO: There's no need to use Indexted8 image. Try thresholding.
void appDrawChar(TRasterGR8P &outImage, TFont::Impl *pimpl, wchar_t charcode)
{
	QImage qimage(70, 70, QImage::Format_ARGB32);
	wchar_t wchararray[1] = {charcode};
	auto dbgstr = QString::fromWCharArray(wchararray);
	std::wstring wstr = {charcode};
	dbgstr = QString::fromStdWString(wstr);
	QPainter painter(&qimage);
	painter.fillRect(qimage.rect(), Qt::white);
	painter.drawText(qimage.rect(), Qt::AlignCenter, QString::fromStdWString(wstr));
	painter.end();

//	const QVector<QRgb> colortable = {qRgb(255,255,255), qRgb(0, 0, 0)};
//	QImage qimage8 = qimage.convertToFormat(QImage::Format_Indexed8, colortable);
	QImage qimage8 = qimage.convertToFormat(QImage::Format_Indexed8, Qt::MonoOnly);

//	QLabel* lbl = new QLabel();
//	lbl->setPixmap(QPixmap::fromImage(qimage8));
//	lbl->show();

	outImage = TRasterGR8P(70,70);
	TRasterP rimage = rasterFromQImage(qimage8);
	outImage = TRasterGR8P(rimage);
	outImage->getLx();
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
			for(const QString eachStyle: m_database.styles(eachFamily)){
				m_currentQFont->setStyleName(eachStyle);
				break;
			}
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
}

//---------------------------------------------------------

void TFontManager::setTypeface(const wstring typeface)
{
	m_pimpl->m_currentQFont->setStyleName(QString::fromStdWString(typeface));
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
	return m_pimpl->m_currentQFont->styleName().toStdWString();
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
