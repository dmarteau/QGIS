/***************************************************************************
                              qgswmslayerinfos.h

  Layer informations
  ------------------------------------
  begin                : September 26 , 2022
  copyright            : (C) 2022 by René-Luc d'Hont and David Marteau
  email                : rldhont at 3liz doc com
         dmarteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSWMSLAYERINFOS_H
#define QGSWMSLAYERINFOS_H

#include "qgsrectangle.h"
#include "qgsmaplayerserverproperties.h"

class QgsServerInterface;
class QgsProject;

/**
 * \ingroup server
 * \brief WMS Layer infos
 */

namespace QgsWms
{
  struct layerInfos
  {
    //! QGIS layer id
    QString id;

    //! WMS layer name
    QString name;

    //! WMS layer title
    QString title;

    //! WMS layer abstract
    QString abstract;

    //! WMS layer keywords
    QStringList keywords;

    //! WMS layer WGS84 bounding rectangle (can be empty)
    QgsRectangle wgs84BoundingRect;

    //! WMS layer CRS extents (can be empty)
    QMap<QString, QgsRectangle> crsExtents;

    //! WMS layer styles
    QStringList styles;

    //! WMS layer legend URL
    QString legendUrl;

    //! WMS layer legend URL format
    QString legendUrlFormat;

    //! WMS layer is queryable
    bool queryable = false;

    //! WMS layer maximum scale (if negative, no maximum scale is defined)
    double maxScale = -1.0;

    //! WMS layer minimum scale (if negative, no maximum scale is defined)
    double minScale = -1.0;

    //! WMS layer dataUrl
    QString dataUrl;

    //! WMS layer attribution
    QString attribution;

    //! WMS layer attribution URL
    QString attributionUrl;

    //! WMS layer metadata URLs
    QList<QgsMapLayerServerProperties::MetadataUrl> metadataUrls;

    //! QGIS layer type
    QgsMapLayerType type;
  };

  /**
   * Returns the WMS layers definition to build WMS capabilities
   * \since QGIS 3.28.0
   */
  QMap< QString, layerInfos > getWmsLayerInfos(
    QgsServerInterface *serverIface,
    const QgsProject *project,
    const QList<QgsCoordinateReferenceSystem> &outputCrsList );

  /**
   * Returns a map with crs authid as key and the transformed extent
   * \since QGIS 3.28.0
   */
  QMap< QString, QgsRectangle > transformExtentToCrsList(
    const QgsRectangle &extent,
    const QgsCoordinateReferenceSystem &source,
    const QList<QgsCoordinateReferenceSystem> &destinations,
    const QgsCoordinateTransformContext &context );

  QgsRectangle transformExtent(
    const QgsRectangle &extent,
    const QgsCoordinateReferenceSystem &source,
    const QgsCoordinateReferenceSystem &destination,
    const QgsCoordinateTransformContext &context,
    const bool &ballparkTransformsAreAppropriate = false );

} // namespace QgsWms

#endif
