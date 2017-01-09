/***************************************************************************
                              qgsmaprendererjobproxy.cpp
                              -------------------
  begin                : 
  copyright            :
  email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"
#include "qgslegendrenderer.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerlegend.h"
#include "qgsmaptopixel.h"
#include "qgsproject.h"
#include "qgsrasteridentifyresult.h"
#include "qgsrasterlayer.h"
#include "qgsrasterrenderer.h"
#include "qgsscalecalculator.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsmapserviceexception.h"
#include "qgssldconfigparser.h"
#include "qgssymbol.h"
#include "qgsrenderer.h"
#include "qgspaintenginehack.h"
#include "qgsogcutils.h"
#include "qgsfeature.h"
#include "qgseditorwidgetregistry.h"
#include "qgsaccesscontrol.h"
#include "qgsmaprenderercustompainterjob.h"
#include "qgsmaprendererparalleljob.h"
#include "qgsserversettings.h"
#include "qgsmapendererjobproxy.h"

#include <QPainter>

namespace QgsWms
{


QgsMapRendererJobProxy::QgsMapRendererJobProxy(
  bool parallelRendering
  , int maxThreads
  , QgsAccessControl* accessControl
)
    :
    mParallelRendering( parallelRendering )
    , mAccessControl( accessControl )
{
  if ( mParallelRendering )
  {
    QgsApplication::setMaxThreads( maxThreads );
    QgsMessageLog::logMessage( QStringLiteral( "Parallel rendering activated with %1 threads" ).arg( maxThreads ), QStringLiteral( "server" ), QgsMessageLog::INFO );
  }
  else
  {
    QgsMessageLog::logMessage( QStringLiteral( "Parallel rendering deactivated" ), QStringLiteral( "server" ), QgsMessageLog::INFO );
  }
}

void QgsMapRendererJobProxy::render( const QgsMapSettings& mapSettings, QImage* image )
{
  if ( mParallelRendering )
  {
    QgsMapRendererParallelJob renderJob( mapSettings );
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    renderJob.setFeatureFilterProvider( mAccessControl );
#endif
    renderJob.start();
    renderJob.waitForFinished();
    *image = renderJob.renderedImage();
    mPainter.reset( new QPainter( image ) );
  }
  else
  {
    mPainter.reset( new QPainter( image ) );
    QgsMapRendererCustomPainterJob renderJob( mapSettings, mPainter.data() );
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    renderJob.setFeatureFilterProvider( mAccessControl );
#endif
    renderJob.renderSynchronously();
  }
}


QPainter* QgsMapRendererJobProxy::takePainter()
{
  return mPainter.take();
}

} // namespace qgsws
