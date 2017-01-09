/***************************************************************************
                              qgsmaprendererjobproxy.h
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


namespace QgsWms
{

class QgsMapRendererJobProxy
{
  public:

    /** Constructor.
     * @param accessControl Does not take ownership of QgsAccessControl
     */
    QgsMapRendererJobProxy(
      bool parallelRendering
      , int maxThreads
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      , QgsAccessControl* accessControl
#endif
    );

    /** Sequential or parallel map rendering according to qsettings.
      * @param mapSettings passed to MapRendererJob
      * @param the rendered image
      */
    void render( const QgsMapSettings& mapSettings, QImage* image );

    /** Take ownership of the painter used for rendering.
      * @return painter
      */
    QPainter* takePainter();

  private:
    bool mParallelRendering;
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsAccessControl* mAccessControl;
#endif
    QScopedPointer<QPainter> mPainter;
};


}
