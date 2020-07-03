// Copyright 2020, Xmodal
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <QObject>
#include <QQmlPropertyMap>

// the Facade objects requires that the object it aliases emits a valueChanged(const QString &key, const QVariant &value) signal when any of its QProperties is changed.
// once created, the Facade will have a copy of the QObject's properties, but they are not linked yet. to link them, use:

// connect generator changes to facade
// QObject::connect(alias, &Generator::valueChanged, fadcade, &Facade::updateValueRelay);

// connect facade changes to generator
// QObject::connect(facade, &Facade::valueChanged, alias, &Generator::updateValue);

// this unfortunately can't happen in Facade's constructor because QObject doesn't have a valueChanged() signal by default


class Facade : public QQmlPropertyMap
{
    Q_OBJECT
public:
    Facade(QObject *alias);
private:
    bool flagDebug = false;
public slots:
    void updateValueRelay(const QString &key, const QVariant &value);
};
