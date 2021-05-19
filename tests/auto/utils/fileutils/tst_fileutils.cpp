/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include <QtTest>
#include <QDebug>

#include <utils/fileutils.h>

//TESTED_COMPONENT=src/libs/utils
using namespace Utils;

class tst_fileutils : public QObject
{
    Q_OBJECT

public:

private slots:
    void initTestCase();
    void parentDir_data();
    void parentDir();
    void isChildOf_data();
    void isChildOf();
    void fileName_data();
    void fileName();
    void calcRelativePath_data();
    void calcRelativePath();
    void relativePath_specials();
    void relativePath_data();
    void relativePath();
    void fromToString_data();
    void fromToString();

private:
    QTemporaryDir tempDir;
    QString rootPath;
};

static void touch(const QDir &dir, const QString &filename)
{
    QFile file(dir.absoluteFilePath(filename));
    file.open(QIODevice::WriteOnly);
    file.close();
}

void tst_fileutils::initTestCase()
{
    // initialize test for tst_fileutiles::relativePath*()
    QVERIFY(tempDir.isValid());
    rootPath = tempDir.path();
    QDir dir(rootPath);
    dir.mkpath("a/b/c/d");
    dir.mkpath("a/x/y/z");
    dir.mkpath("a/b/x/y/z");
    dir.mkpath("x/y/z");
    touch(dir, "a/b/c/d/file1.txt");
    touch(dir, "a/x/y/z/file2.txt");
    touch(dir, "a/file3.txt");
    touch(dir, "x/y/file4.txt");
}

void tst_fileutils::parentDir_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("parentPath");
    QTest::addColumn<QString>("expectFailMessage");

    QTest::newRow("empty path") << "" << "" << "";
    QTest::newRow("root only") << "/" << "" << "";
    QTest::newRow("//") << "//" << "" << "";
    QTest::newRow("/tmp/dir") << "/tmp/dir" << "/tmp" << "";
    QTest::newRow("relative/path") << "relative/path" << "relative" << "";
    QTest::newRow("relativepath") << "relativepath" << "." << "";

    // Windows stuff:
#ifdef Q_OS_WIN
    QTest::newRow("C:/data") << "C:/data" << "C:/" << "";
    QTest::newRow("C:/") << "C:/" << "" << "";
    QTest::newRow("//./com1") << "//./com1" << "//." << "";
    QTest::newRow("//?/path") << "//?/path" << "/" << "Qt 4 cannot handle this path.";
    QTest::newRow("/Global?\?/UNC/host") << "/Global?\?/UNC/host" << "/Global?\?/UNC/host"
                                        << "Qt 4 cannot handle this path.";
    QTest::newRow("//server/directory/file")
            << "//server/directory/file" << "//server/directory" << "";
    QTest::newRow("//server/directory") << "//server/directory" << "//server" << "";
    QTest::newRow("//server") << "//server" << "" << "";
#endif
}

void tst_fileutils::parentDir()
{
    QFETCH(QString, path);
    QFETCH(QString, parentPath);
    QFETCH(QString, expectFailMessage);

    FilePath result = FilePath::fromString(path).parentDir();
    if (!expectFailMessage.isEmpty())
        QEXPECT_FAIL("", expectFailMessage.toUtf8().constData(), Continue);
    QCOMPARE(result.toString(), parentPath);
}

void tst_fileutils::isChildOf_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("childPath");
    QTest::addColumn<bool>("result");

    QTest::newRow("empty path") << "" << "/tmp" << false;
    QTest::newRow("root only") << "/" << "/tmp" << true;
    QTest::newRow("/tmp/dir") << "/tmp" << "/tmp/dir" << true;
    QTest::newRow("relative/path") << "relative" << "relative/path" << true;
    QTest::newRow("/tmpdir") << "/tmp" << "/tmpdir" << false;
    QTest::newRow("same") << "/tmp/dir" << "/tmp/dir" << false;

    // Windows stuff:
#ifdef Q_OS_WIN
    QTest::newRow("C:/data") << "C:/" << "C:/data" << true;
    QTest::newRow("C:/") << "" << "C:/" << false;
    QTest::newRow("//./com1") << "/" << "//./com1" << true;
    QTest::newRow("//?/path") << "/" << "//?/path" << true;
    QTest::newRow("/Global?\?/UNC/host") << "/Global?\?/UNC/host"
                                        << "/Global?\?/UNC/host/file" << true;
    QTest::newRow("//server/directory/file")
            << "//server/directory" << "//server/directory/file" << true;
    QTest::newRow("//server/directory")
            << "//server" << "//server/directory" << true;
#endif
}

void tst_fileutils::isChildOf()
{
    QFETCH(QString, path);
    QFETCH(QString, childPath);
    QFETCH(bool, result);

    bool res = FilePath::fromString(childPath).isChildOf(FilePath::fromString(path));
    QCOMPARE(res, result);
}

void tst_fileutils::fileName_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<int>("components");
    QTest::addColumn<QString>("result");

    QTest::newRow("empty 1") << "" << 0 << "";
    QTest::newRow("empty 2") << "" << 1 << "";
    QTest::newRow("basic") << "/foo/bar/baz" << 0 << "baz";
    QTest::newRow("2 parts") << "/foo/bar/baz" << 1 << "bar/baz";
    QTest::newRow("root no depth") << "/foo" << 0 << "foo";
    QTest::newRow("root full") << "/foo" << 1 << "/foo";
    QTest::newRow("root included") << "/foo/bar/baz" << 2 << "/foo/bar/baz";
    QTest::newRow("too many parts") << "/foo/bar/baz" << 5 << "/foo/bar/baz";
    QTest::newRow("windows root") << "C:/foo/bar/baz" << 2 << "C:/foo/bar/baz";
    QTest::newRow("smb share") << "//server/share/file" << 2 << "//server/share/file";
    QTest::newRow("no slashes") << "foobar" << 0 << "foobar";
    QTest::newRow("no slashes with depth") << "foobar" << 1 << "foobar";
    QTest::newRow("multiple slashes 1") << "/foo/bar////baz" << 0 << "baz";
    QTest::newRow("multiple slashes 2") << "/foo/bar////baz" << 1 << "bar////baz";
    QTest::newRow("multiple slashes 3") << "/foo////bar/baz" << 2 << "/foo////bar/baz";
    QTest::newRow("single char 1") << "/a/b/c" << 0 << "c";
    QTest::newRow("single char 2") << "/a/b/c" << 1 << "b/c";
    QTest::newRow("single char 3") << "/a/b/c" << 2 << "/a/b/c";
    QTest::newRow("slash at end 1") << "/a/b/" << 0 << "";
    QTest::newRow("slash at end 2") << "/a/b/" << 1 << "b/";
    QTest::newRow("slashes at end 1") << "/a/b//" << 0 << "";
    QTest::newRow("slashes at end 2") << "/a/b//" << 1 << "b//";
    QTest::newRow("root only 1") << "/" << 0 << "";
    QTest::newRow("root only 2") << "/" << 1 << "/";
}

void tst_fileutils::fileName()
{
    QFETCH(QString, path);
    QFETCH(int, components);
    QFETCH(QString, result);
    QCOMPARE(FilePath::fromString(path).fileNameWithPathComponents(components), result);
}

void tst_fileutils::calcRelativePath_data()
{
    QTest::addColumn<QString>("absolutePath");
    QTest::addColumn<QString>("anchorPath");
    QTest::addColumn<QString>("result");

    QTest::newRow("empty") << "" << "" << "";
    QTest::newRow("leftempty") << "" << "/" << "";
    QTest::newRow("rightempty") << "/" << "" << "";
    QTest::newRow("root") << "/" << "/" << "";
    QTest::newRow("simple1") << "/a" << "/" << "a";
    QTest::newRow("simple2") << "/" << "/a" << "..";
    QTest::newRow("simple3") << "/a" << "/a" << "";
    QTest::newRow("extraslash1") << "/a/b/c" << "/a/b/c" << "";
    QTest::newRow("extraslash2") << "/a/b/c" << "/a/b/c/" << "";
    QTest::newRow("extraslash3") << "/a/b/c/" << "/a/b/c" << "";
    QTest::newRow("normal1") << "/a/b/c" << "/a/x" << "../b/c";
    QTest::newRow("normal2") << "/a/b/c" << "/a/x/y" << "../../b/c";
    QTest::newRow("normal3") << "/a/b/c" << "/x/y" << "../../a/b/c";
}

void tst_fileutils::calcRelativePath()
{
    QFETCH(QString, absolutePath);
    QFETCH(QString, anchorPath);
    QFETCH(QString, result);
    QString relativePath = Utils::FilePath::calcRelativePath(absolutePath, anchorPath);
    QCOMPARE(relativePath, result);
}

void tst_fileutils::relativePath_specials()
{
    QString path = FilePath::fromString("").relativePath(FilePath::fromString("")).toString();
    QCOMPARE(path, "");
}

void tst_fileutils::relativePath_data()
{
    QTest::addColumn<QString>("relative");
    QTest::addColumn<QString>("anchor");
    QTest::addColumn<QString>("result");

    QTest::newRow("samedir") << "/" << "/" << "";
    QTest::newRow("dir2dir_1") << "a/b/c/d" << "a/x/y/z" << "../../../b/c/d";
    QTest::newRow("dir2dir_2") << "a/b" <<"a/b/c" << "..";
    QTest::newRow("file2file_1") << "a/b/c/d/file1.txt" << "a/file3.txt" << "b/c/d/file1.txt";
    QTest::newRow("dir2file_1") << "a/b/c" << "a/x/y/z/file2.txt" << "../../../b/c";
    QTest::newRow("file2dir_1") << "a/b/c/d/file1.txt" << "x/y" << "../../a/b/c/d/file1.txt";
}

void tst_fileutils::relativePath()
{
    QFETCH(QString, relative);
    QFETCH(QString, anchor);
    QFETCH(QString, result);
    FilePath actualPath = FilePath::fromString(rootPath + "/" + relative)
                              .relativePath(FilePath::fromString(rootPath + "/" + anchor));
    QCOMPARE(actualPath.toString(), result);
}

void tst_fileutils::fromToString_data()
{
    QTest::addColumn<QString>("scheme");
    QTest::addColumn<QString>("host");
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("full");

    QTest::newRow("s0") << "" << "" << "" << "";
    QTest::newRow("s1") << "" << "" << "/" << "/";
    QTest::newRow("s2") << "" << "" << "a/b/c/d" << "a/b/c/d";
    QTest::newRow("s3") << "" << "" << "/a/b" << "/a/b";

    QTest::newRow("s4")
        << "docker" << "1234abcdef" << "/bin/ls" << "docker://1234abcdef/bin/ls";

    QTest::newRow("s5")
        << "docker" << "1234" << "/bin/ls" << "docker://1234/bin/ls";

    // This is not a proper URL.
    QTest::newRow("s6")
        << "docker" << "1234" << "somefile" << "docker://1234/./somefile";

    // Local Windows paths:
    QTest::newRow("w1") << "" << "" << "C:/data" << "C:/data";
    QTest::newRow("w2") << "" << "" << "C:/" << "C:/";
    QTest::newRow("w3") << "" << "" << "//./com1" << "//./com1";
    QTest::newRow("w4") << "" << "" << "//?/path" << "//?/path";
    QTest::newRow("w5") << "" << "" << "/Global?\?/UNC/host" << "/Global?\?/UNC/host";
    QTest::newRow("w6") << "" << "" << "//server/dir/file" << "//server/dir/file";
    QTest::newRow("w7") << "" << "" << "//server/dir" << "//server/dir";
    QTest::newRow("w8") << "" << "" << "//server" << "//server";

    // Not supported yet: "Remote" windows. Would require use of e.g.
    // FileUtils::isRelativePath with support from the remote device
    // identifying itself as Windows.

    //  Actual   (filePath.path()): "/C:/data"
    //  Expected (path)           : "C:/data"

    //QTest::newRow("w9") << "scheme" << "server" << "C:/data"
    //    << "scheme://server/C:/data";
}

void tst_fileutils::fromToString()
{
    QFETCH(QString, full);
    QFETCH(QString, scheme);
    QFETCH(QString, host);
    QFETCH(QString, path);

    FilePath filePath = FilePath::fromString(full);

    QCOMPARE(filePath.toString(), full);

    QCOMPARE(filePath.scheme(), scheme);
    QCOMPARE(filePath.host(), host);
    QCOMPARE(filePath.path(), path);


    FilePath copy = filePath;
    copy.setHost(host);
    QCOMPARE(copy.toString(), full);

    copy.setScheme(scheme);
    QCOMPARE(copy.toString(), full);

    copy.setPath(path);
    QCOMPARE(copy.toString(), full);
}

QTEST_APPLESS_MAIN(tst_fileutils)
#include "tst_fileutils.moc"
