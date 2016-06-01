#ifndef OPENTOONZ_QIMAGEUTIL_H
#define OPENTOONZ_QIMAGEUTIL_H
#include "trastercm.h"
#include "tstroke.h"
#include "tmathutil.h"

#include <QImage>

//QImage DVAPI rasterToQImage(const TRasterP &ras, bool premultiplied = true, bool mirrored = true);
////	TRaster32P DVAPI rasterFromQImage(QImage image, bool premultiply = true, bool mirror = true);
TRasterP rasterFromQImage(const QImage &image);
TStroke* convertPointsToStroke(std::vector<TThickPoint> points);
std::vector<TStroke*> convertQPainterPathToStrokes(const QPainterPath pathes);


#endif //OPENTOONZ_QIMAGEUTIL_H
