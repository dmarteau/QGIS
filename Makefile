# 
# Compile QGIS on OSX with qt4 and python 2.7 on MacPort platform
#
# INSTALL Note:
#
# Installation of /Applications/QGIS.app/Contents/Resources/python/PyQt4/Qsci.so is broken
# and one must replace with /opt/local/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/site-packages/PyQt4/Qsci.so which has correct linking
#
# install qwt60 +qt4 with option -f (force): even if it is marked as incompatible with qwt61 the
#
# Required python packages for Metasearch plugin:
# py27-future
# py27-requests
# py27-jinja2
# py27-pygments
#

.PHONY: build install clean clobber 

INSTALL_PATH=/Applications

BUILDDIR:=build

#PYTHON_FRAMEWORK:=/opt/local/Library/Frameworks/Python.framework/Versions/2.7

JOBS:=-j6

export PATH := $(PATH):/opt/local/libexec/qt4/bin

configure:
	mkdir -p $(BUILDDIR)
	rm -rf $(BUILDDIR)/CMakeCache.txt
	cd $(BUILDDIR) && cmake $(CMAKE_OPTS) \
		-DCMAKE_INSTALL_PREFIX:PATH=$(INSTALL_PATH) \
		-DCMAKE_INSTALL_PREFIX:PATH=/Applications \
		-DWITH_SERVER=ON \
		-DWITH_PYSPATIALITE=ON \
		-DPYTHON_INCLUDE_PATH:PATH=/opt/local/include/python2.7 \
		-DPYTHON_LIBRARY:FILEPATH=/opt/local/lib/libpython2.7.dylib \
		-DEXPAT_INCLUDE_DIR:PATH=/opt/local/include \
		-DEXPAT_LIBRARY:FILEPATH=/opt/local/lib/libexpat.dylib \
		-DSQLITE3_INCLUDE_DIR:PATH=/opt/local/include \
		-DSQLITE3_LIBRARY:FILEPATH=/opt/local/lib/libsqlite3.dylib \
		-DQT_QMAKE_EXECUTABLE:FILEPATH=/opt/local/libexec/qt4/bin/qmake \
		-DQWT_INCLUDE_DIR:PATH=/opt/local/libexec/qt4/include/qwt \
		-DQWT_LIBRARY:FILEPATH=/opt/local/libexec/qt4/lib/libqwt.dylib \
		-DQCA_INCLUDE_DIR:PATH=/opt/local/libexec/qt4/include/QtCrypto \
		-DQCA_LIBRARY:FILEPATH=/opt/local/libexec/qt4/lib/libqca.dylib \
		-DQSCINTILLA_INCLUDE_DIR:PATH=/opt/local/libexec/qt4/include \
		-DQSCINTILLA_LIBRARY:FILEPATH=/opt/local/libexec/qt4/lib/libqscintilla2_qt4.dylib \
	..


build:
	cd $(BUILDDIR) && $(MAKE) $(JOBS)

install:
	cd $(BUILDDIR) && $(MAKE) install

clean:
	cd $(BUILDDIR) && $(MAKE) clean

clobber:
	rm -rf $(BUILDDIR)

run:
	open $(INSTALL_PATH)/QGIS.app


