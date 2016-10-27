#include <thread>

#include <boost/asio.hpp>

#include "../contrib/gtest/gtest.h"
#include "../protocol/redis.h"
/*#include "../server/Server.h"
#include "../server/Storage.h"
#include "../server/Commands.h"*/


using boost::asio::ip::tcp;


TEST(RedisValue, Construct) {
    RedisValue integer = 10;
    RedisValue string = "abcd";
    RedisValue error = RedisError("Permission denied");
    RedisValue null = RedisNull();
    RedisValue bulk = RedisBulkString("1\0\\1");

    RedisValue array = std::vector<RedisValue>{integer, string, error, null, bulk};
}

TEST(WriteRedisValue, Int) {
    std::shared_ptr<StringWriter> writer(new StringWriter(1024));
    RedisValue i = 10;
    WriteRedisValue(writer, i);
    writer->flush();


    EXPECT_STREQ(":10\r\n", writer->result.c_str());

    writer->result.clear();
    RedisValue j = -5;
    WriteRedisValue(writer, j);
    writer->flush();

    EXPECT_STREQ(":-5\r\n", writer->result.c_str());

    writer->result.clear();
    RedisValue max = 9223372036854775807;
    WriteRedisValue(writer, max);
    writer->flush();

    EXPECT_STREQ(":9223372036854775807\r\n", writer->result.c_str());

    writer->result.clear();
    RedisValue min = -9223372036854775807;
    WriteRedisValue(writer, min);
    writer->flush();

    EXPECT_STREQ(":-9223372036854775807\r\n", writer->result.c_str());
}

TEST(WriteRedisValue, String) {
    std::shared_ptr<StringWriter> writer(new StringWriter(1024));
    RedisValue str = "abcd";
    WriteRedisValue(writer, str);
    writer->flush();

    EXPECT_STREQ("+abcd\r\n", writer->result.c_str());

    writer->result.clear();
    str = "123";
    WriteRedisValue(writer, str);
    writer->flush();

    EXPECT_STREQ("+123\r\n", writer->result.c_str());
}

TEST(WriteRedisValue, Error) {
    std::shared_ptr<StringWriter> writer(new StringWriter(1024));
    RedisValue str = RedisError("Error message");
    WriteRedisValue(writer, str);
    writer->flush();

    EXPECT_STREQ("-Error message\r\n", writer->result.c_str());
}

TEST(WriteRedisValue, Null) {
    std::shared_ptr<StringWriter> writer(new StringWriter(1024));
    RedisValue null = RedisNull();
    WriteRedisValue(writer, null);
    writer->flush();

    EXPECT_STREQ("$-1\r\n", writer->result.c_str());
}

TEST(WriteRedisValue, Array) {
    std::shared_ptr<StringWriter> writer(new StringWriter(1024));
    RedisValue integer = 10;
    RedisValue string = "abcd";
    RedisValue error = RedisError("Permission denied");
    RedisValue null = RedisNull();

    RedisValue array = std::vector<RedisValue>{integer, string, error, null};
    WriteRedisValue(writer, array);
    writer->flush();

    EXPECT_STREQ("*4\r\n:10\r\n+abcd\r\n-Permission denied\r\n$-1\r\n", writer->result.c_str());
}

TEST(WriteRedisValue, BulkString) {
    std::shared_ptr<StringWriter> writer(new StringWriter(1024));

    RedisValue bulk = RedisBulkString("\t\r\n");
    WriteRedisValue(writer, bulk);
    writer->flush();

    EXPECT_STREQ("$3\r\n\t\r\n\r\n", writer->result.c_str());
}

TEST(WriteRedisValue, BufOverflow) {
    std::shared_ptr<StringWriter> writer(new StringWriter(3));

    RedisValue data = RedisBulkString("123456");
    WriteRedisValue(writer, data);

    EXPECT_STREQ("$6\r\n123456\r\n", writer->result.c_str());
}


TEST(ReadRedisValue, Int) {
    RedisValue val;
    std::shared_ptr<StringReader> reader(new StringReader());

    reader->input = ":10\r\n";
    ReadRedisValue(reader, val);
    EXPECT_EQ(10, boost::get<int64_t>(val));

    reader->input = ":-5\r\n";
    ReadRedisValue(reader, val);
    EXPECT_EQ(-5, boost::get<int64_t>(val));

    reader->input = ":9223372036854775807\r\n";
    ReadRedisValue(reader, val);
    EXPECT_EQ(9223372036854775807, boost::get<int64_t>(val));

    reader->input = ":-9223372036854775807\r\n";
    ReadRedisValue(reader, val);
    EXPECT_EQ(-9223372036854775807, boost::get<int64_t>(val));

    try {
        reader->input = ":-9999999999999999999\r\n";
        ReadRedisValue(reader, val);
    } catch (std::invalid_argument &e) {
        EXPECT_STREQ(e.what(), "Integer overflow");
    } catch (...) {
        FAIL() << "Expected std::invalid_argument of overflow integer";
    }
}

TEST(ReadRedisValue, Null) {
    RedisValue val;
    std::shared_ptr<StringReader> reader(new StringReader());

    reader->input = "$-1\r\n";
    ReadRedisValue(reader, val);
    EXPECT_EQ(REDIS_NULL, val.which());
}

TEST(ReadRedisValue, String) {
    RedisValue val;
    std::shared_ptr<StringReader> reader(new StringReader());

    reader->input = "+OK\r\n";
    ReadRedisValue(reader, val);
    EXPECT_STREQ("OK", boost::get<std::string>(val).c_str());
}

TEST(ReadRedisValue, Error) {
    RedisValue val;
    std::shared_ptr<StringReader> reader(new StringReader());

    reader->input = "-Error message\r\n";
    ReadRedisValue(reader, val);
    EXPECT_STREQ("Error message", boost::get<RedisError>(val).msg.c_str());

    reader->input = "-123\r\n";
    ReadRedisValue(reader, val);
    EXPECT_STREQ("123", boost::get<RedisError>(val).msg.c_str());
}

TEST(ReadRedisValue, Array) {
    RedisValue val;
    std::shared_ptr<StringReader> reader(new StringReader());

    reader->input = "*4\r\n:10\r\n+abcd\r\n-Permission denied\r\n$-1\r\n";
    ReadRedisValue(reader, val);
    EXPECT_EQ(10, boost::get<int64_t>(boost::get<std::vector<RedisValue>>(val)[0]));
    EXPECT_STREQ("abcd", boost::get<std::string>(boost::get<std::vector<RedisValue>>(val)[1]).c_str());
    EXPECT_STREQ("Permission denied", boost::get<RedisError>(boost::get<std::vector<RedisValue>>(val)[2]).msg.c_str());
    EXPECT_EQ(REDIS_NULL, boost::get<std::vector<RedisValue>>(val)[3].which());
}


TEST(ReadRedisValue, BulkString) {
    RedisValue val;
    std::shared_ptr<StringReader> reader(new StringReader());

    reader->input = "$3\r\n\t\r\n\r\n";
    ReadRedisValue(reader, val);
    EXPECT_STREQ("\t\r\n", boost::get<RedisBulkString>(val).data.c_str());

    reader->input = "$0\r\n\r\n";
    ReadRedisValue(reader, val);
    EXPECT_STREQ("", boost::get<RedisBulkString>(val).data.c_str());
}

extern template class SocketWriter<boost::asio::local::stream_protocol::socket>;
extern template class SocketReader<boost::asio::local::stream_protocol::socket>;
TEST(RedisServer, Socket) {
    typedef boost::asio::local::stream_protocol::socket Socket;

    boost::asio::io_service service;


    std::shared_ptr<Socket> writer_socket(new Socket(service));
    std::shared_ptr<Socket> reader_socket(new Socket(service));

    boost::asio::local::connect_pair(*writer_socket, *reader_socket);

    std::shared_ptr<SocketWriter<Socket>> writer(new SocketWriter<Socket>(writer_socket ,1024));
    std::shared_ptr<SocketReader<Socket>> reader(new SocketReader<Socket>(reader_socket, 41));

    RedisValue integer = 10;
    RedisValue string = "abcd";
    RedisValue error = RedisError("Permission denied");
    RedisValue null = RedisNull();
    RedisValue array = std::vector<RedisValue>{integer, string, error, null};

    WriteRedisValue(writer, array);
    writer->flush();
    RedisValue val;
    ReadRedisValue(reader, val);

    EXPECT_EQ(10, boost::get<int64_t>(boost::get<std::vector<RedisValue>>(val)[0]));
    EXPECT_STREQ("abcd", boost::get<std::string>(boost::get<std::vector<RedisValue>>(val)[1]).c_str());
    EXPECT_STREQ("Permission denied", boost::get<RedisError>(boost::get<std::vector<RedisValue>>(val)[2]).msg.c_str());
    EXPECT_EQ(REDIS_NULL, boost::get<std::vector<RedisValue>>(val)[3].which());
}

//TEST(WriteRedisValue, Socket) {
//    int fd[2];
//    socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
//
//    OldSocketWriter writer(fd[0], 1024);
//    OldSocketReader reader(fd[1], 41);
//
//    RedisValue integer = 10;
//    RedisValue string = "abcd";
//    RedisValue error = RedisError("Permission denied");
//    RedisValue null = RedisNull();
//    RedisValue array = std::vector<RedisValue>{integer, string, error, null};
//
//    WriteRedisValue(&writer, array);
//    writer.flush();
//    RedisValue val;
//    ReadRedisValue(&reader, &val);
//
//    EXPECT_EQ(10, boost::get<int64_t>(boost::get<std::vector<RedisValue>>(val)[0]));
//    EXPECT_STREQ("abcd", boost::get<std::string>(boost::get<std::vector<RedisValue>>(val)[1]).c_str());
//    EXPECT_STREQ("Permission denied", boost::get<RedisError>(boost::get<std::vector<RedisValue>>(val)[2]).msg.c_str());
//    EXPECT_EQ(REDIS_NULL, boost::get<std::vector<RedisValue>>(val)[3].which());
//}

/*

TEST(RedisServer, AcceptConnection) {
    TestServer s(6376, 1);

    std::thread t([&] {
        s.serve();
    });

    LocalSocket soccli1;
    soccli1.connect_(6376);

    t.join();

    ASSERT_EQ(1, s.countConn_);
}


TEST(RedisServer, ReadData) {
    TestServer s(6376, 1);
    char inp[4];
    std::thread t([&] {
        s.serve();
        s.out_->getData(inp, 3);
    });

    LocalSocket soccli;
    soccli.connect_(6376);
    soccli.write_(std::string("abc"));

    t.join();

    ASSERT_STREQ("abc", inp);
}


TEST(RedisServer, WriteData) {
    TestServer s(6376, 1);
    std::string * inp;

    std::thread t([&] {
        s.serve();
        char str[] = "abc";
        s.out_->sendData(str, 3);
    });

    LocalSocket soccli;
    soccli.connect_(6376);
    inp = soccli.read_(3);

    t.join();

    ASSERT_STREQ("abc", inp->c_str());
}


TEST(RedisStorage, Set) {
    Storage table;
    Set s(&table);
    RedisValue cmd = RedisBulkString("SET");
    RedisValue key = RedisBulkString("a");
    RedisValue value = RedisBulkString("b");
    RedisValue val = std::vector<RedisValue> {cmd, key, value};

    RedisValue result = s.exec(val);
    EXPECT_EQ(SET, s.name());
    EXPECT_STREQ("b", table.getPtrOfStorage()->operator[]("a").c_str());
    EXPECT_STREQ("OK", boost::get<std::string>(result).c_str());

    key = RedisBulkString("b");
    value = RedisBulkString("abc");
    val = std::vector<RedisValue> {cmd, key, value};

    result = s.exec(val);
    EXPECT_STREQ("abc", table.getPtrOfStorage()->operator[]("b").c_str());
    EXPECT_STREQ("OK", boost::get<std::string>(result).c_str());
}


TEST(RedisStorage, Get) {
    Storage table;

    table.getPtrOfStorage()->operator[]("a") = "b";

    Get g(&table);
    RedisValue cmd = RedisBulkString("GET");
    RedisValue key = RedisBulkString("a");
    RedisValue val = std::vector<RedisValue> {cmd, key};

    RedisValue result = g.exec(val);

    EXPECT_EQ(GET, g.name());
    EXPECT_STREQ("b", boost::get<RedisBulkString>(result).data.c_str());
}*/
