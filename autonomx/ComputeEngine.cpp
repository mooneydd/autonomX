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

#include <algorithm>
#include <chrono>
#include <QDebug>
#include <QThread>

#include "ComputeEngine.h"
#include "AppModel.h"

ComputeEngine::ComputeEngine(QSharedPointer<QList<QSharedPointer<Generator>>> generatorsList, QSharedPointer<QHash<int, QSharedPointer<Generator>>> generatorsHashMap) : randomUniform(0.0, 1.0) {
    if(flagDebug) {
        std::chrono::nanoseconds now = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::system_clock::now().time_since_epoch()
        );

        qDebug() << "constructor (ComputeEngine):\tt = " << now.count() << "\tid = " << QThread::currentThreadId();
    }

    this->generatorsList = generatorsList;
    this->generatorsHashMap = generatorsHashMap;
}

ComputeEngine::~ComputeEngine() {
    if(flagDebug) {
        std::chrono::nanoseconds now = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::system_clock::now().time_since_epoch()
        );// get generator

        qDebug() << "destructor (ComputeEngine):\tt = " << now.count() << "\tid = " << QThread::currentThreadId();
    }
}

void ComputeEngine::receiveOscData(int id, QVariantList data) {
    if(!generatorsHashMap->contains(id)) {
        throw std::runtime_error("generator does not exist");
    }
    QSharedPointer<Generator> generator = generatorsHashMap->value(id);

    QList<QVariant> dataAsList = data;

    int argumentsTotal = data.size();
    int argumentsValid = 0;

    for(int i = 0; i < generator->getInputRegionSet()->getRegionCount(); i++) {
        // set default to 0
        double input = 0;
        // check that the message's list is long enough
        if(i < data.size()) {
            // check the message's type can be cast to double
            QMetaType::Type type = (QMetaType::Type) dataAsList.at(i).type();
            if(type == QMetaType::Float || type == QMetaType::Double || type == QMetaType::Int || type == QMetaType::Long) {
                input = dataAsList.at(i).toDouble();
                argumentsValid++;
            }
        }
        // write to input
        generator->getInputRegionSet()->getRegion(i)->writeMirroredIntensity(input);
    }

    if(flagDebug) {
        std::chrono::nanoseconds now = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::system_clock::now().time_since_epoch()
        );

        qDebug() << "receiveOscData (ComputeEngine):\tt = " << now.count() << "\tid = " << QThread::currentThreadId() << "\tgenid = " << id << "\tdata = " << data << "\t(" << argumentsValid << " of " << argumentsTotal << " valid)";
    }
}

void ComputeEngine::addGenerator(QSharedPointer<Generator> generator) {
    if(flagDebug) {
        std::chrono::nanoseconds now = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::system_clock::now().time_since_epoch()
        );// get generator

        qDebug() << "addGenerator (ComputeEngine):\tt = " << now.count() << "\tid = " << QThread::currentThreadId();
    }

    generatorsList->append(generator);
    generatorsHashMap->insert(generator->getID(), generator);
}

void ComputeEngine::removeGenerator(QSharedPointer<Generator> generator) {
    if(flagDebug) {
        std::chrono::nanoseconds now = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::system_clock::now().time_since_epoch()
        );// get generator

        qDebug() << "removeGenerator (ComputeEngine):\tt = " << now.count() << "\tid = " << QThread::currentThreadId();
    }

    generatorsList->removeOne(generator);
    generatorsHashMap->remove(generator->getID());
}

void ComputeEngine::start() {
    // this is called from an external thread initially and allows this object's thread to pick up the loop statement through its event queue
    // having a singleshot timer with time zero executes as soon as possible
    QTimer timer;
    timer.setTimerType(Qt::PreciseTimer);
    timer.singleShot(0, this, &ComputeEngine::loop);
}

void ComputeEngine::loop() {

    if(flagDisableProcessing) {
        return;
    }

    double millisCompute;   // time in nanoseconds taken by computation
    double millisLastFrame; // time in nanoseconds since last frame

    // compute time since last frame, with exception if this is the first frame
    if(firstFrame) {
        firstFrame = false;
        millisLastFrame = 1.0 / frequency * 1000.0;
    } else {
        millisLastFrame = elapsedTimer.nsecsElapsed() / 1000000.0;
    }

    // restart the timer to measure frame time and compute time
    elapsedTimer.restart();
    elapsedTimer.start();

    // apply input values
    for(QList<QSharedPointer<Generator>>::iterator it = generatorsList->begin(); it != generatorsList->end(); it++) {
        (*it)->applyInputRegion();
    }

    // do the computation
    for(QList<QSharedPointer<Generator>>::iterator it = generatorsList->begin(); it != generatorsList->end(); it++) {
        (*it)->computeIteration(1.0 / frequency);
    }

    // apply output values
    for(QList<QSharedPointer<Generator>>::iterator it = generatorsList->begin(); it != generatorsList->end(); it++) {
        (*it)->applyOutputRegion();
    }

    // write to history
    for(QList<QSharedPointer<Generator>>::iterator it = generatorsList->begin(); it != generatorsList->end(); it++) {
        double historyLatest = 0;
        for(int i = 0; i < (*it)->getOutputRegionSet()->getRegionCount(); i++) {
            historyLatest += (*it)->getOutputRegionSet()->getRegion(i)->getIntensity();
        }
        if(flagDummyOutputMonitor) {
            // random output
            historyLatest = randomUniform(randomGenerator);
        } else {
            // dumb averaging
            historyLatest /= (*it)->getOutputRegionSet()->getRegionCount();
            // saturation polynomial
            historyLatest = (1.0 - pow(1.0 - historyLatest, 3));
        }
        // write value
        (*it)->writeHistoryLatest(historyLatest);
        // flip refresher to force update on QML side
        (*it)->flipHistoryRefresher();
    }

    // send output messages to osc engine
    for(QList<QSharedPointer<Generator>>::iterator it = generatorsList->begin(); it != generatorsList->end(); it++) {
        QList<QVariant> outputs;
        for(int i = 0; i < (*it)->getOutputRegionSet()->getRegionCount(); i++) {
            double value = flagDummyOutputMonitor ? randomUniform(randomGenerator) : (*it)->getOutputRegionSet()->getRegion(i)->getIntensity();
            if(flagCastOutputToFloat) {
                outputs.append((float)value);
            } else {
                outputs.append(value);
            }
        }
        emit sendOscData((*it)->getID(), outputs);
    }

    // measure the time used to do the computation
    millisCompute = elapsedTimer.nsecsElapsed() / 1000000.0;

    QTimer timer;
    timer.setTimerType(Qt::PreciseTimer);
    timer.singleShot((int) std::min<double>(1.0 / frequency * 1000.0, std::max<double>(1.0 / frequency * 1000.0 - millisCompute, 0)), this, &ComputeEngine::loop);

    if(flagDebug) {
        std::chrono::nanoseconds now = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::system_clock::now().time_since_epoch()
        );

        qDebug() << "loop (ComputeEngine):\t\tt = " << now.count() << "\tid = " << QThread::currentThreadId() << "\trefresh (ms) = " << millisLastFrame << "\tcompute (ms) = " << millisCompute;
    }
}
