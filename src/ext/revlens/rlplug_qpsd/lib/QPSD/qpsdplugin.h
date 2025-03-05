/*
Copyright (c) 2012-2013 Ronie Martinez (ronmarti18@gmail.com)
Copyright (c) 2013 Yuezhao Huang (huangezhao@gmail.com)
All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU Lesser General Public License for more
details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301  USA
*/

#ifndef QPSDPLUGIN_H
#define QPSDPLUGIN_H

#include <QtGui/QImageIOPlugin>

#include "RlpExtrevlens/QPSD/Global.h"


class EXTREVLENS_QPSD_API QPsdPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "QPsdPlugin.json")
    
public:
    QPsdPlugin(QObject *parent = 0);
    ~QPsdPlugin();
    QStringList keys() const;
    Capabilities capabilities(QIODevice *device,
                    const QByteArray &format) const;
    QImageIOHandler *create(QIODevice *device,
                               const QByteArray &format = QByteArray()) const;
};

#endif // QPSDPLUGIN_H

