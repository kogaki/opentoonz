#include "tvrender/qimageutil.h"

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
				colorTable.append(qRgb(i, i, i));
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


TStroke* convertPointsToStroke(std::vector<TThickPoint> points){
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
	return stroke;
}

std::vector<TStroke*> convertQPainterPathToStrokes(const QPainterPath pathes){
	std::vector<TStroke*> strokes;
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

