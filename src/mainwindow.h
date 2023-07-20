#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QThread>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTableWidget>
#include <leveldb/db.h>

#include "ui_mainwindow.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenuBar;
QT_END_NAMESPACE

class MyApp : public QMainWindow, private Ui::MainWindow {
    Q_OBJECT

public:
    MyApp(int argc, char *argv[]);
    ~MyApp();

private:
    inline void addLevelDb(const QString &levelDbDir);
    QMenuBar *menuBar;
    QThread workerThread;

Q_SIGNALS:
    void operate(const QString &levelDbDir);
};

class Worker : public QObject {
    Q_OBJECT

public Q_SLOTS:
    void doWork(const QString &levelDbDir) {
        leveldb::DB *db;
        leveldb::Options dbOptions;
        dbOptions.create_if_missing = false;
        leveldb::Status status = leveldb::DB::Open(dbOptions, levelDbDir.toStdString(), &db);
        QMap<QByteArray, QByteArray> map;
        if (!status.ok()) {
            qCritical() << status.ToString();
            emit resultReady(map);
            return;
        }
        const auto it = db->NewIterator(leveldb::ReadOptions());
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            auto key = QByteArray::fromRawData(it->key().data(), it->key().size());
            if (key.isEmpty()) {
                key = QByteArrayLiteral("<empty>");
            }
            auto value = QByteArray::fromRawData(it->value().data(), it->value().size());
            if (value.isEmpty()) {
                value = QByteArrayLiteral("<empty>");
            }
            map[key] = value;
        }
        emit resultReady(map);
    }

Q_SIGNALS:
    void resultReady(const QMap<QByteArray, QByteArray> &results);
};

#endif // MAINWINDOW_H