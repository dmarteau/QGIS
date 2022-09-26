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

#include "qgsserverprojectutils.h"
#include "qgscoordinatetransform.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgswmslayerinfos.h"
#include "qgsmessagelog.h"
#include "qgsserverinterface.h"
#include "qgsmaplayerstylemanager.h"

#include <algorithm>

namespace QgsWms
{
  QgsRectangle transformExtent(
    const QgsRectangle &extent,
    const QgsCoordinateReferenceSystem &source,
    const QgsCoordinateReferenceSystem &destination,
    const QgsCoordinateTransformContext &context,
    const bool &ballparkTransformsAreAppropriate )
  {
    QgsCoordinateTransform transformer { source, destination, context };
    transformer.setBallparkTransformsAreAppropriate( ballparkTransformsAreAppropriate );
    // Transform extent and do not catch exception
    return transformer.transformBoundingBox( extent );
  }

  QMap< QString, QgsRectangle > transformExtentToCrsList(
    const QgsRectangle &extent,
    const QgsCoordinateReferenceSystem &source,
    const QList<QgsCoordinateReferenceSystem> &destinations,
    const QgsCoordinateTransformContext &context )
  {
    QMap< QString, QgsRectangle > crsExtents;
    if ( extent.isEmpty() )
    {
      return crsExtents;
    }
    for ( const QgsCoordinateReferenceSystem &destination : destinations )
    {
      // Transform extent and do not catch exception
      QgsCoordinateTransform crsTransform { source, destination, context };
      crsExtents[ destination.authid() ] = crsTransform.transformBoundingBox( extent );
    }
    return crsExtents;
  }


  bool setBoundingRect(
    const QgsProject *project,
    layerInfos &pLayer,
    QgsMapLayer *ml,
    const QgsRectangle &wmsExtent,
    const QgsCoordinateReferenceSystem &wgs84,
    const QList<QgsCoordinateReferenceSystem> &outputCrsList )
  {
    QgsRectangle layerExtent = ml->extent();
    if ( layerExtent.isEmpty() )
    {
      // if the extent is empty (not only Null), use the wms extent
      // defined in the project...
      if ( wmsExtent.isNull() )
      {
        // or the CRS extent otherwise
        layerExtent = ml->crs().bounds();
      }
      else
      {
        layerExtent = QgsRectangle( wmsExtent );
        if ( ml->crs() != project->crs() )
        {
          // If CRS is different transform it to layer's CRS
          try
          {
            layerExtent = transformExtent( wmsExtent, project->crs(), ml->crs(), project->transformContext() );
          }
          catch ( QgsCsException &cse )
          {
            QgsMessageLog::logMessage( QStringLiteral( "Error transforming extent for layer %1: %2" ).arg( ml->name() ).arg( cse.what() ), QStringLiteral( "Server" ), Qgis::MessageLevel::Warning );
            return false;
          }
        }
      }

      // Now we have a layer Extent we need the WGS84 bounding rectangle
      try
      {
        pLayer.wgs84BoundingRect = transformExtent( layerExtent, ml->crs(), wgs84, project->transformContext(), true );
      }
      catch ( const QgsCsException &cse )
      {
        QgsMessageLog::logMessage( QStringLiteral( "Error transforming extent for layer %1: %2" ).arg( ml->name() ).arg( cse.what() ), QStringLiteral( "Server" ), Qgis::MessageLevel::Warning );
        return false;
      }
    }
    else
    {
      pLayer.wgs84BoundingRect = ml->wgs84Extent();
    }

    try
    {
      pLayer.crsExtents = transformExtentToCrsList(
                            layerExtent, ml->crs(), outputCrsList, project->transformContext()
                          );
    }
    catch ( QgsCsException &cse )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Error transforming extent for layer %1: %2" ).arg( ml->name() ).arg( cse.what() ), QStringLiteral( "Server" ), Qgis::MessageLevel::Warning );
      return false;
    }

    return true;
  }

  // ===================================
  // Get wms layer infos
  // ===================================
  QMap< QString, layerInfos > getWmsLayerInfos(
    QgsServerInterface *serverIface,
    const QgsProject *project,
    const QList<QgsCoordinateReferenceSystem> &outputCrsList )
  {
    QMap< QString, layerInfos > wmsLayers;
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsAccessControl *accessControl = serverIface->accessControls();
#else
    ( void )serverIface;
#endif

    bool useLayerIds = QgsServerProjectUtils::wmsUseLayerIds( *project );
    const QStringList restrictedLayers = QgsServerProjectUtils::wmsRestrictedLayers( *project );
    const QgsRectangle wmsExtent = QgsServerProjectUtils::wmsExtent( *project );
    const QgsCoordinateReferenceSystem wgs84 = QgsCoordinateReferenceSystem::fromOgcWmsCrs( geoEpsgCrsAuthId() );

    for ( QgsMapLayer *ml : project->mapLayers() )
    {
      if ( !ml || restrictedLayers.contains( ml->name() ) ) //unpublished layer
      {
        continue;
      }

#ifdef HAVE_SERVER_PYTHON_PLUGINS
      if ( accessControl && !accessControl->layerReadPermission( ml ) )
      {
        continue;
      }
#endif

      layerInfos pLayer;
      pLayer.id = ml->id();

      // Calculate layer extents for the WMS output CRSes list
      // First define if the layer have an extent
      if ( ml->type() == QgsMapLayerType::VectorLayer )
      {
        QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( ml );
        if ( vLayer )
        {
          if ( vLayer->wkbType() != QgsWkbTypes::NoGeometry )
          {
            // If the layer has an extent get it and the WGS84 bounding rectangle
            if ( !setBoundingRect( project, pLayer, ml, wmsExtent, wgs84, outputCrsList ) )
            {
              continue;
            }
          }
        }
      }
      else
      {
        // If the layer has an extent get it and the WGS84 bounding rectangle
        if ( !setBoundingRect( project, pLayer, ml, wmsExtent, wgs84, outputCrsList ) )
        {
          continue;
        }
      }

      // layer type
      pLayer.type = ml->type();
      // layer wms name
      pLayer.name = ml->name();
      if ( useLayerIds )
      {
        pLayer.name = ml->id();
      }
      else if ( !ml->shortName().isEmpty() )
      {
        pLayer.name = ml->shortName();
      }
      // layer title
      pLayer.title = ml->title();
      if ( pLayer.title.isEmpty() )
      {
        pLayer.title = ml->name();
      }
      // layer abstract
      pLayer.abstract = ml->abstract();
      // layer is queryable
      pLayer.queryable = ml->flags().testFlag( QgsMapLayer::Identifiable );
      // layer keywords
      if ( !ml->keywordList().isEmpty() )
      {
        pLayer.keywords = ml->keywordList().split( ',' );
      }
      // layer styles
      pLayer.styles = ml->styleManager()->styles();
      // layer legend URL
      pLayer.legendUrl = ml->legendUrl();
      // layer legend URL format
      pLayer.legendUrlFormat = ml->legendUrlFormat();
      // layer min/max scales
      if ( ml->hasScaleBasedVisibility() )
      {
        pLayer.maxScale = ml->maximumScale();
        pLayer.minScale = ml->minimumScale();
      }
      // layer data URL
      pLayer.dataUrl = ml->dataUrl();
      // layer attribution
      pLayer.attribution = ml->attribution();
      pLayer.attributionUrl = ml->attributionUrl();
      // layer metadata URLs
      pLayer.metadataUrls = ml->serverProperties()->metadataUrls();

      wmsLayers[pLayer.id] = pLayer;
    }

    return wmsLayers;
  }


} // namespace QgsWms
