// Unit tests of the GUI-free helpers used by FaustLive:
//  - src/Utilities/utilities.cpp : compilation options parsing (used by
//    FLSessionManager::getFactoryArgv), file helpers, isStringInt (preferences)
//  - src/MenusAndDialogs/SimpleParser.cpp : FaustWeb targets JSON parser (export)
//
// Known bugs are kept visible with QEXPECT_FAIL: fix the code, then remove the
// QEXPECT_FAIL line (the test then reports XPASS until you do).

#include <QtTest>
#include <QTemporaryDir>

#include "utilities.h"
#include "SimpleParser.h"
#include "FJUI.h"

class TestUtilities : public QObject {
    Q_OBJECT

   private:
    // Splits options the way getFactoryArgv() does: count first, then parse.
    static QStringList splitOptions(const QString& options)
    {
        QString copy = options;
        QStringList res;
        int n = get_numberParameters(options);
        for (int i = 0; i < n; i++) {
            res << QString::fromStdString(parse_compilationParams(copy));
        }
        return res;
    }

   private slots:

    //---------------- compilation options
    void compilationOptions_data()
    {
        QTest::addColumn<QString>("options");
        QTest::addColumn<QStringList>("expected");

        QTest::newRow("empty") << "" << QStringList();
        QTest::newRow("blank") << "   " << QStringList();
        QTest::newRow("single") << "-double" << QStringList{"-double"};
        QTest::newRow("key value") << "-vec -vs 32" << QStringList{"-vec", "-vs", "32"};
        QTest::newRow("extra blanks") << "  -vec   -lv 1  " << QStringList{"-vec", "-lv", "1"};
        QTest::newRow("dash in value") << "-I /tmp/my-dir -double" << QStringList{"-I", "/tmp/my-dir", "-double"};
        QTest::newRow("leading value") << "foo -double" << QStringList{"foo", "-double"};
        QTest::newRow("value then key") << "-vs 32 bar -vec" << QStringList{"-vs", "32", "bar", "-vec"};
        QTest::newRow("tabs") << "-vec\t-vs\t16" << QStringList{"-vec", "-vs", "16"};
        QTest::newRow("trailing value") << "-ftz 2" << QStringList{"-ftz", "2"};
    }

    void compilationOptions()
    {
        QFETCH(QString, options);
        QFETCH(QStringList, expected);
        QCOMPARE(splitOptions(options), expected);
    }

    void parseConsumesOptions()
    {
        QString options = "-vec -vs 32";
        QCOMPARE(QString::fromStdString(parse_compilationParams(options)), QString("-vec"));
        QCOMPARE(QString::fromStdString(parse_compilationParams(options)), QString("-vs"));
        QCOMPARE(QString::fromStdString(parse_compilationParams(options)), QString("32"));
        QCOMPARE(QString::fromStdString(parse_compilationParams(options)), QString(""));
    }

    //---------------- isStringInt (port and optimisation level fields)
    void stringInt_data()
    {
        QTest::addColumn<QString>("word");
        QTest::addColumn<bool>("expected");
        QTest::newRow("zero") << "0" << true;
        QTest::newRow("port") << "5510" << true;
        QTest::newRow("negative") << "-1" << false;
        QTest::newRow("letters") << "12a" << false;
        QTest::newRow("blank") << " 1" << false;
        QTest::newRow("empty") << "" << false;
        QTest::newRow("non ascii") << QString::fromUtf8("1\u00e9") << false;
    }

    void stringInt()
    {
        QFETCH(QString, word);
        QFETCH(bool, expected);
        QCOMPARE(isStringInt(word.toLatin1().constData()), expected);
    }

    //---------------- file helpers
    void writeThenPathToContent()
    {
        // FLSessionManager saves the DSP source with writeFile() in its SHAFolder,
        // then libfaust reads it back: non ASCII characters must survive.
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QString path = dir.filePath("test.dsp");
        QString content = QString::fromUtf8("declare name \"Écho\";\nprocess = _ : *(0.5); // gain ½\n");
        writeFile(path, content);
        QCOMPARE(pathToContent(path), content);
    }

    void pathToContentMissingFile()
    {
        QTemporaryDir dir;
        QCOMPARE(pathToContent(dir.filePath("missing.dsp")), QString(""));
    }

    void readFileUtf8()
    {
        QTemporaryDir dir;
        QString path = dir.filePath("page.html");
        QString content = QString::fromUtf8("<p>Écho</p>");
        writeFile(path, content);
        QCOMPARE(readFile(path), content);
    }

    //---------------- external commands (sound2faust, post-compilation script, snapshots)
#ifndef Q_OS_WIN
    void executeInstructionRuns()
    {
        // executeInstruction() used to give the whole command line to QProcess::start(program, {}),
        // so nothing ever ran and it always reported success.
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QString file = dir.filePath("with blank.txt");
        QString error;
        QVERIFY2(executeInstruction("touch \"" + file + "\"", error), qPrintable(error));
        QVERIFY(QFileInfo(file).exists());
    }

    void executeInstructionErrors()
    {
        QString error;
        QVERIFY(!executeInstruction("false", error));
        QVERIFY(error.contains("exit code 1"));
        error.clear();
        QVERIFY(!executeInstruction("/nonexistent/faustlive-command", error));
        QVERIFY(!error.isEmpty());
        QVERIFY(!executeInstruction("   ", error));
    }

    void executeProgramArguments()
    {
        QTemporaryDir dir;
        QString file = dir.filePath("a b \"c\".txt");
        QString error;
        QVERIFY2(executeProgram("touch", QStringList{file}, error), qPrintable(error));
        QVERIFY(QFileInfo(file).exists());
    }

    void tarRoundTrip()
    {
        // FLSessionManager::createSnapshot / snapshotRestoration
        QTemporaryDir dir;
        QString snapshot = dir.filePath("My Snapshot");
        QVERIFY(QDir().mkpath(snapshot + "/Windows/FLW-1"));
        writeFile(snapshot + "/Windows/FLW-1/FLW-1.dsp", "process = _;");
        QString error;
        QVERIFY2(tarFolder(snapshot, error), qPrintable(error));
        QVERIFY(QFileInfo(snapshot + ".tar").exists());
        deleteDirectoryAndContent(snapshot);
        QVERIFY(!QFileInfo(snapshot).exists());
        // The archive is relative: it can be extracted anywhere
        QString elsewhere = dir.filePath("Temp");
        QVERIFY(QDir().mkpath(elsewhere));
        QVERIFY2(untarFolder(snapshot + ".tar", elsewhere, error), qPrintable(error));
        QCOMPARE(pathToContent(elsewhere + "/My Snapshot/Windows/FLW-1/FLW-1.dsp"), QString("process = _;"));
    }
#endif

    void deleteDirectory()
    {
        QTemporaryDir dir;
        QString folder = dir.filePath("SHA key");
        QVERIFY(QDir().mkpath(folder + "/sub"));
        writeFile(folder + "/sub/file.dsp", "process = _;");
        deleteDirectoryAndContent(folder);
        QVERIFY(!QFileInfo(folder).exists());
        QVERIFY(QFileInfo(dir.path()).exists());
        // An empty path must not mean the current folder
        deleteDirectoryAndContent("");
        QVERIFY(QDir::current().exists());
    }

    void touchFolderDate()
    {
        // FLSessionManager::cleanSHAFolder removes the least recently used SHA folders
        QTemporaryDir dir;
        QString folder = dir.filePath("SHA");
        QVERIFY(QDir().mkpath(folder));
        QDateTime before = QFileInfo(folder).lastModified();
        QTest::qWait(1100);
        touchFolder(folder);
        QFileInfo after(folder);
        after.refresh();
        QVERIFY(after.lastModified() > before);
        QVERIFY(QDir(folder).entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden).isEmpty());
    }

    //---------------- FJUI (Connections.jc of a window, renamed on duplicate / snapshot import)
    void connectionsUpdate()
    {
        QTemporaryDir dir;
        std::string file = dir.filePath("Connections.jc").toStdString();
        std::list<std::pair<std::string, std::string> > connections;
        connections.push_back(std::make_pair("FLW-1:out_0", "system:playback_1"));
        connections.push_back(std::make_pair("FLW-12:out_0", "FLW-1:in 0"));
        connections.push_back(std::make_pair("system:capture_1", "FLW-2:in_0"));
        FJUI::saveConnections(file.c_str(), connections);
        QCOMPARE(FJUI::recallConnections(file.c_str()), connections);

        std::map<std::string, std::string> changes;
        changes["FLW-1"] = "FLW-3";
        changes["FLW-2"] = "FLW-4";
        FJUI::update(file.c_str(), changes);

        std::list<std::pair<std::string, std::string> > expected;
        expected.push_back(std::make_pair("FLW-3:out_0", "system:playback_1"));
        expected.push_back(std::make_pair("FLW-12:out_0", "FLW-3:in 0"));
        expected.push_back(std::make_pair("system:capture_1", "FLW-4:in_0"));
        QCOMPARE(FJUI::recallConnections(file.c_str()), expected);
    }

    void connectionsUpdateMissingFile()
    {
        QTemporaryDir dir;
        std::string file = dir.filePath("Connections.jc").toStdString();
        FJUI::update(file.c_str(), std::map<std::string, std::string>());
        QVERIFY(FJUI::recallConnections(file.c_str()).empty());
    }

    //---------------- SimpleParser (FaustWeb /targets answer)
    void parseTargets()
    {
        const char* json =
            "{\"android\": [\"android\"],\n"
            " \"linux\" : [\"alsa-gtk\", \"jack-qt\"] ,\n"
            " \"osx\": [\"coreaudio-qt\"]}";
        const char* p = json;
        std::vector<std::string> platforms;
        std::map<std::string, std::vector<std::string>> targets;
        QVERIFY(parseOperatingSystemsList(p, platforms, targets));
        QCOMPARE(platforms.size(), size_t(3));
        QCOMPARE(QString::fromStdString(platforms[1]), QString("linux"));
        QCOMPARE(targets["linux"].size(), size_t(2));
        QCOMPARE(QString::fromStdString(targets["linux"][1]), QString("jack-qt"));
        QCOMPARE(QString::fromStdString(targets["osx"][0]), QString("coreaudio-qt"));
    }

    void parseTargetsMalformed_data()
    {
        QTest::addColumn<QString>("json");
        QTest::newRow("missing bracket") << "{\"linux\": \"jack-qt\"}";
        QTest::newRow("unterminated string") << "{\"linux\": [\"jack-qt]}";
        QTest::newRow("missing brace") << "{\"linux\": [\"jack-qt\"]";
        QTest::newRow("not json") << "<html>502 Bad Gateway</html>";
        QTest::newRow("empty") << "";
    }

    void parseTargetsMalformed()
    {
        QFETCH(QString, json);
        QByteArray bytes = json.toUtf8();
        const char* p = bytes.constData();
        std::vector<std::string> platforms;
        std::map<std::string, std::vector<std::string>> targets;
        QVERIFY(!parseOperatingSystemsList(p, platforms, targets));
    }
};

QTEST_GUILESS_MAIN(TestUtilities)
#include "tst_utilities.moc"
