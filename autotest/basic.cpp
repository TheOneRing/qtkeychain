#include <QtTest>

#include "qtkeychain/keychain.h"

namespace
{
    QString serviceKey = QStringLiteral("QtKeychainTest");

    QByteArray generateRandomString(qsizetype size)
    {
        std::vector<quint32> buffer(size, 0);
        QRandomGenerator::global()->fillRange(buffer.data(), size);
        return QByteArray(reinterpret_cast<char *>(buffer.data()), static_cast<int>(size * sizeof(quint32))).toBase64(QByteArray::Base64UrlEncoding).mid(0, size);
    }

}
class BasicTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void test_data()
    {
        QTest::addColumn<QByteArray>("password");
        QTest::newRow("normal password") << QByteArrayLiteral("this is a password");
        QTest::newRow("1k") << generateRandomString(1000);
        QTest::newRow("2k") << generateRandomString(2000);
        QTest::newRow("3k") << generateRandomString(3000);

    }

    void test()
    {
        QFETCH(QByteArray, password);
        {
            QKeychain::WritePasswordJob writeJob(serviceKey);
            writeJob.setKey(serviceKey);
            writeJob.setBinaryData(password);
            QSignalSpy writeSpy(&writeJob, &QKeychain::WritePasswordJob::finished);
            writeJob.start();
            writeSpy.wait();
            QCOMPARE(writeJob.error(), QKeychain::NoError);
        }
        {
            QKeychain::ReadPasswordJob readJob(serviceKey);
            readJob.setKey(serviceKey);
            QSignalSpy readSpy(&readJob, &QKeychain::ReadPasswordJob::finished);
            readJob.start();
            readSpy.wait();
            QCOMPARE(readJob.error(), QKeychain::NoError);
            QCOMPARE(readJob.binaryData(), password);
        }
        {
            QKeychain::DeletePasswordJob deleteJob(serviceKey);
            deleteJob.setKey(serviceKey);
            QSignalSpy deleteSpy(&deleteJob, &QKeychain::DeletePasswordJob::finished);
            deleteJob.start();
            deleteSpy.wait();
            QCOMPARE(deleteJob.error(), QKeychain::NoError);
        }
    }
};

QTEST_MAIN(BasicTest)
#include "basic.moc"
