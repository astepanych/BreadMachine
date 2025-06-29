#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <appcore.h>

#ifdef Q_OS_ANDROID
#include <QtAndroid>

bool requestAndroidPermissions(){
    //Request requiered permissions at runtime

    const QVector<QString> permissions({"android.permission.WRITE_EXTERNAL_STORAGE",
                                        "android.permission.READ_EXTERNAL_STORAGE"});

    for(const QString &permission : permissions){
        auto result = QtAndroid::checkPermission(permission);
        qDebug()<<permission;
        qDebug()<<(int)result;
        if(result == QtAndroid::PermissionResult::Denied){
            auto resultHash = QtAndroid::requestPermissionsSync(QStringList({permission}));
            qDebug()<<permission;
            qDebug()<<(int)resultHash[permission];
            if(resultHash[permission] == QtAndroid::PermissionResult::Denied){
                return false;
            }
        }
    }
    return true;
}
#endif


int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QGuiApplication app(argc, argv);
#ifdef Q_OS_ANDROID
    requestAndroidPermissions();
#endif



    AppCore appCore;
    // Установка кодировки для вывода (например, UTF-8)
        QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    qmlRegisterType<AppCore>("com.ics.demo", 1, 0, "AppCore");

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QLatin1String("appCore"), &appCore);
    engine.rootContext()->setContextProperty(QLatin1String("modelMain"), appCore.logModel());
    engine.rootContext()->setContextProperty(QLatin1String("modelPrograms"), appCore.modelWM());
    engine.rootContext()->setContextProperty(QLatin1String("paramsWorkMode"), appCore.paramsWM());


    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);


    return app.exec();
}
