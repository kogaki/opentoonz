#include <tcurves.h>
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
		std::cout<<"not close...."<<std::endl;
		points.push_back((lastPoint + firstPoint) * 0.5);
		points.push_back(firstPoint);
	}

	// TODO: maybe bug
//	if(points.size() % 2 == 0){
//		points.push_back(firstPoint);
//	}


	stroke->reshape(&(points[0]), points.size());
	stroke->setSelfLoop(true);

	for(int i=0;i <stroke->getChunkCount();i++){
		auto point = stroke->getChunk(i);
		std::cout<<"("<<point->getP0()<<","<<point->getP1()<<","<<point->getP2()<<")"<<std::endl;
	}
	return stroke;
}

void push_back_debug(TThickPoint point, std::vector<TThickPoint>& points){
	std::cout<<"pb:"<<point.x<<","<<point.y<<std::endl;
	points.push_back(point);
}

std::vector<TStroke*> convertQPainterPathToStrokes(const QPainterPath pathes){
	std::vector<TStroke*> strokes;
	std::vector<TThickPoint> points;
	for(int i=0; i<pathes.elementCount();){
		auto element = pathes.elementAt(i);
		TThickPoint point = TThickPoint(element.x, element.y, 0);
		if(element.isMoveTo()){ // stroke start
			std::cout<<i<<">move"<<element.x<<","<<element.y<<std::endl;
			if(!points.empty()){
				auto lastElement = pathes.elementAt(i-1);
				auto lastPoint = TThickPoint(lastElement.x, lastElement.y, 0);
				push_back_debug(lastPoint, points);
				strokes.push_back(convertPointsToStroke(points));
			}
			points.clear();
//			points.push_back(point);
		}else if (element.isCurveTo()){
			std::cout<<i<<">curve"<<element.x<<","<<element.y<<std::endl;
			auto lastElement = pathes.elementAt(i-1);
			if(lastElement.isLineTo() || lastElement.isMoveTo()){
				std::cout<<i<<">line to curve"<<std::endl;
				auto lastPoint = TThickPoint(lastElement.x, lastElement.y, 0);
//				push_back_debug(lastPoint + (point - lastPoint)*0.5, points);
//				push_back_debug(lastPoint + (point - lastPoint)*0.75, points);
//				push_back_debug(lastPoint + (point - lastPoint)*0.5, points);


//				push_back_debug(point, points);
				auto curveElement1 = pathes.elementAt(i+1);
				assert(curveElement1.type == QPainterPath::ElementType::CurveToDataElement);
				auto curvePoint1 = TThickPoint(curveElement1.x, curveElement1.y, 0);
				auto curveElement2 = pathes.elementAt(i+2);
				assert(curveElement2.type == QPainterPath::ElementType::CurveToDataElement);
				auto curvePoint2 = TThickPoint(curveElement2.x, curveElement2.y, 0);
				i+=2;


				if(i<pathes.elementCount()-1) {
					auto nextElement = pathes.elementAt(i + 1);
					auto nextPoint = TThickPoint(nextElement.x, nextElement.y, 0);
					push_back_debug(lastPoint, points);
					push_back_debug((point-lastPoint)*1.5 + lastPoint, points); // I really don't know why should I x1.5
				}

			}

		}else if(element.isLineTo()){
			std::cout<<i<<">line"<<element.x<<","<<element.y<<std::endl;
			std::cout<<points.size()<<std::endl;
			auto lastElement = pathes.elementAt(i-1);
			auto lastPoint = TThickPoint(lastElement.x, lastElement.y, 0);
			if(lastElement.type == QPainterPath::ElementType::CurveToDataElement){
//				lastElement = pathes.elementAt(i-2);
//				lastPoint = TThickPoint(lastElement.x, lastElement.y, 0);
				push_back_debug(lastPoint, points);
//				push_back_debug(lastPoint + (point - lastPoint)*0.25, points);
				push_back_debug(lastPoint + (point - lastPoint)*0.5, points);
//				push_back_debug(lastPoint + (point - lastPoint)*0.75, points);

			}else if (lastElement.isLineTo()){
				push_back_debug(lastPoint, points);
				push_back_debug(lastPoint + (point - lastPoint)*0.25, points);
				push_back_debug(lastPoint + (point - lastPoint)*0.5, points);
				push_back_debug(lastPoint + (point - lastPoint)*0.75, points);

			}


//			if(i<pathes.elementCount()-1){
//				auto nextElement = pathes.elementAt(i+1);
//				auto nextPoint = TThickPoint(nextElement.x, nextElement.y, 0);
//				if(nextElement.isCurveTo()){
//					push_back_debug(lastPoint, points);
//					push_back_debug(lastPoint + (point - lastPoint)*0.25, points);
//					push_back_debug(lastPoint + (point - lastPoint)*0.5, points);
//					push_back_debug(lastPoint + (point - lastPoint)*0.75, points);
//
//				}else {
//					push_back_debug(lastPoint, points);
//					push_back_debug(lastPoint + (point - lastPoint)*0.25, points);
//					push_back_debug(lastPoint + (point - lastPoint)*0.5, points);
//					push_back_debug(lastPoint + (point - lastPoint)*0.75, points);
//
//				}
//
//
////
////				push_back_debug(point + (nextPoint-point)*0.5, points);
////				push_back_debug(point + (nextPoint-point)*0.75, points);
//			}
		}
		i++;
	}
	if(!points.empty()){
		auto lastElement = pathes.elementAt(pathes.elementCount()-1);
		auto lastPoint = TThickPoint(lastElement.x, lastElement.y, 0);
		points.push_back(lastPoint);
		strokes.push_back(convertPointsToStroke(points));
	}
	return strokes;
}

std::vector<TStroke*> convertQPainterPathToStrokes2(const QPainterPath pathes){
	std::vector<TStroke*> strokes;
	std::vector<TThickPoint> points;
	std::vector<TThickQuadratic*> controlPoints;
	for(int i=0; i<pathes.elementCount();){
		auto element = pathes.elementAt(i);
		TThickPoint point = TThickPoint(element.x, element.y, 0);
		if(element.isMoveTo()){ // stroke start
//			if(!points.empty()){
//				strokes.push_back(convertPointsToStroke(points));
//			}
			if(!controlPoints.empty()){
				TThickQuadratic* lastControlPoint = new TThickQuadratic(*controlPoints.front());
				controlPoints.push_back(lastControlPoint);
				TStroke* stroke = TStroke::create(controlPoints);
				stroke->setSelfLoop(true);
				strokes.push_back(stroke);
			}
			controlPoints.clear();
			points.clear();
		}else if (element.isCurveTo()){
			std::cout<<"curve"<<std::endl;
			std::vector<TThickPoint> curvePoints;
			curvePoints.push_back(point);
			for(int j=0; j<2; j++){
				i++;
				auto subElement = pathes.elementAt(i);
				assert(subElement.type == QPainterPath::ElementType::CurveToDataElement);
				TThickPoint subPoint = TThickPoint(subElement.x, subElement.y, 0);
				curvePoints.push_back(subPoint);
			}
			TThickQuadratic* controlPoint = new TThickQuadratic(curvePoints[0], curvePoints[1], curvePoints[2]);
			controlPoints.push_back(controlPoint);
		}else if(element.isLineTo()){
			std::cout<<i<<">line"<<element.x<<","<<element.y<<std::endl;
			auto lastElement = pathes.elementAt(i-1);
			auto lastPoint = TThickPoint(lastElement.x, lastElement.y, 0);

			// 1/4 point and 3/4 point
			if(i >= 2){
				auto lastLastElement = pathes.elementAt(i-2);
				auto lastLastPoint = TThickPoint(lastLastElement.x, lastLastElement.y, 0);
				TThickQuadratic* firstQuarterControlPoint = new TThickQuadratic(
						lastPoint - (lastPoint - lastLastPoint) * 0.25,
						lastPoint,
						lastPoint + (point - lastPoint)*0.25
				);
				controlPoints.push_back(firstQuarterControlPoint);
			}
			TThickQuadratic *thirdQuarterControlPoint = new TThickQuadratic(
					lastPoint + (point - lastPoint) * 0.25,
					lastPoint + (point - lastPoint) * 0.5,
					lastPoint + (point - lastPoint) * 0.75
			);
			controlPoints.push_back(thirdQuarterControlPoint);
		}
		i++;
	}
	if(!controlPoints.empty()){
		TThickPoint firstPoint = controlPoints.front()->getThickP1();
		TThickPoint lastPoint = controlPoints.back()->getThickP1();
		TThickQuadratic* lastControlPoint = new TThickQuadratic(
				lastPoint + (firstPoint-lastPoint) * 0.25,
				lastPoint + (firstPoint-lastPoint) * 0.5,
				lastPoint + (firstPoint-lastPoint) * 0.75
		);
		controlPoints.push_back(lastControlPoint);
		TStroke* stroke = TStroke::create(controlPoints);
//		stroke->setSelfLoop(true);
		strokes.push_back(stroke);
	}
//	if(!points.empty()){
//		strokes.push_back(convertPointsToStroke(points));
//	}
	return strokes;
}
