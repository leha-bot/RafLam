#include <thread>

#include <boost/asio.hpp>

#include "../contrib/gtest/gtest.h"
#include "../protocol/redis.h"

#include "../rpc/marshaling.h"


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
    protocol::WriteRedisValue(writer, i);
    writer->flush();


    EXPECT_STREQ(":10\r\n", writer->result.c_str());

    writer->result.clear();
    RedisValue j = -5;
    protocol::WriteRedisValue(writer, j);
    writer->flush();

    EXPECT_STREQ(":-5\r\n", writer->result.c_str());

    writer->result.clear();
    RedisValue max = 9223372036854775807;
    protocol::WriteRedisValue(writer, max);
    writer->flush();

    EXPECT_STREQ(":9223372036854775807\r\n", writer->result.c_str());

    writer->result.clear();
    RedisValue min = -9223372036854775807;
    protocol::WriteRedisValue(writer, min);
    writer->flush();

    EXPECT_STREQ(":-9223372036854775807\r\n", writer->result.c_str());
}

TEST(WriteRedisValue, String) {
    std::shared_ptr<StringWriter> writer(new StringWriter(1024));
    RedisValue str = "abcd";
    protocol::WriteRedisValue(writer, str);
    writer->flush();

    EXPECT_STREQ("+abcd\r\n", writer->result.c_str());

    writer->result.clear();
    str = "123";
    protocol::WriteRedisValue(writer, str);
    writer->flush();

    EXPECT_STREQ("+123\r\n", writer->result.c_str());
}

TEST(WriteRedisValue, Error) {
    std::shared_ptr<StringWriter> writer(new StringWriter(1024));
    RedisValue str = RedisError("Error message");
    protocol::WriteRedisValue(writer, str);
    writer->flush();

    EXPECT_STREQ("-Error message\r\n", writer->result.c_str());
}

TEST(WriteRedisValue, Null) {
    std::shared_ptr<StringWriter> writer(new StringWriter(1024));
    RedisValue null = RedisNull();
    protocol::WriteRedisValue(writer, null);
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
    protocol::WriteRedisValue(writer, array);
    writer->flush();

    EXPECT_STREQ("*4\r\n:10\r\n+abcd\r\n-Permission denied\r\n$-1\r\n", writer->result.c_str());
}

TEST(WriteRedisValue, BulkString) {
    std::shared_ptr<StringWriter> writer(new StringWriter(1024));

    RedisValue bulk = RedisBulkString("\t\r\n");
    protocol::WriteRedisValue(writer, bulk);
    writer->flush();

    EXPECT_STREQ("$3\r\n\t\r\n\r\n", writer->result.c_str());
}

TEST(WriteRedisValue, BufOverflow) {
    std::shared_ptr<StringWriter> writer(new StringWriter(3));

    RedisValue data = RedisBulkString("123456");
    protocol::WriteRedisValue(writer, data);

    EXPECT_STREQ("$6\r\n123456\r\n", writer->result.c_str());
}


TEST(ReadRedisValue, Int) {
    RedisValue val;
    std::shared_ptr<StringReader> reader(new StringReader());

    reader->input = ":10\r\n";
    protocol::ReadRedisValue(reader, val);
    EXPECT_EQ(10, boost::get<int64_t>(val));

    reader->input = ":-5\r\n";
    protocol::ReadRedisValue(reader, val);
    EXPECT_EQ(-5, boost::get<int64_t>(val));

    reader->input = ":9223372036854775807\r\n";
    protocol::ReadRedisValue(reader, val);
    EXPECT_EQ(9223372036854775807, boost::get<int64_t>(val));

    reader->input = ":-9223372036854775807\r\n";
    protocol::ReadRedisValue(reader, val);
    EXPECT_EQ(-9223372036854775807, boost::get<int64_t>(val));

    try {
        reader->input = ":-9999999999999999999\r\n";
        protocol::ReadRedisValue(reader, val);
    } catch (std::invalid_argument &e) {
        EXPECT_STREQ(e.what(), "Redis-protocol: Integer overflow");
    } catch (...) {
        FAIL() << "Expected std::invalid_argument of overflow integer";
    }
}

TEST(ReadRedisValue, Null) {
    RedisValue val;
    std::shared_ptr<StringReader> reader(new StringReader());

    reader->input = "$-1\r\n";
    protocol::ReadRedisValue(reader, val);
    EXPECT_EQ(REDIS_NULL, val.which());
}

TEST(ReadRedisValue, String) {
    RedisValue val;
    std::shared_ptr<StringReader> reader(new StringReader());

    reader->input = "+OK\r\n";
    protocol::ReadRedisValue(reader, val);
    EXPECT_STREQ("OK", boost::get<std::string>(val).c_str());
}

TEST(ReadRedisValue, Error) {
    RedisValue val;
    std::shared_ptr<StringReader> reader(new StringReader());

    reader->input = "-Error message\r\n";
    protocol::ReadRedisValue(reader, val);
    EXPECT_STREQ("Error message", boost::get<RedisError>(val).msg.c_str());

    reader->input = "-123\r\n";
    protocol::ReadRedisValue(reader, val);
    EXPECT_STREQ("123", boost::get<RedisError>(val).msg.c_str());
}

TEST(ReadRedisValue, Array) {
    RedisValue val;
    std::shared_ptr<StringReader> reader(new StringReader());

    reader->input = "*4\r\n:10\r\n+abcd\r\n-Permission denied\r\n$-1\r\n";
    protocol::ReadRedisValue(reader, val);
    EXPECT_EQ(10, boost::get<int64_t>(boost::get<std::vector<RedisValue>>(val)[0]));
    EXPECT_STREQ("abcd", boost::get<std::string>(boost::get<std::vector<RedisValue>>(val)[1]).c_str());
    EXPECT_STREQ("Permission denied", boost::get<RedisError>(boost::get<std::vector<RedisValue>>(val)[2]).msg.c_str());
    EXPECT_EQ(REDIS_NULL, boost::get<std::vector<RedisValue>>(val)[3].which());
}


TEST(ReadRedisValue, BulkString) {
    RedisValue val;
    std::shared_ptr<StringReader> reader(new StringReader());

    reader->input = "$3\r\n\t\r\n\r\n";
    protocol::ReadRedisValue(reader, val);
    EXPECT_STREQ("\t\r\n", boost::get<RedisBulkString>(val).data.c_str());

    reader->input = "$0\r\n\r\n";
    protocol::ReadRedisValue(reader, val);
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

    protocol::WriteRedisValue(writer, array);
    writer->flush();
    RedisValue val;
    protocol::ReadRedisValue(reader, val);

    EXPECT_EQ(10, boost::get<int64_t>(boost::get<std::vector<RedisValue>>(val)[0]));
    EXPECT_STREQ("abcd", boost::get<std::string>(boost::get<std::vector<RedisValue>>(val)[1]).c_str());
    EXPECT_STREQ("Permission denied", boost::get<RedisError>(boost::get<std::vector<RedisValue>>(val)[2]).msg.c_str());
    EXPECT_EQ(REDIS_NULL, boost::get<std::vector<RedisValue>>(val)[3].which());
}

TEST(Marshaling, convert_int) {
    std::shared_ptr<StringWriter> writer(new StringWriter(1024));
    RedisValue out;

    std::string f_str = "f";

    int i = 10;
    auto m =  Marshaling<int>(writer, f_str, 1);
    m.convert<int>(i, out);

    protocol::WriteRedisValue(writer, out);
    writer->flush();

    EXPECT_STREQ(":10\r\n", writer->result.c_str());
}

TEST(Marshaling, convert_string) {
    std::shared_ptr<StringWriter> writer(new StringWriter(1024));
    RedisValue out;

    std::string f_str = "f";

    std::string str = "abcd";
    auto n =  Marshaling<int>(writer, f_str, 1);
    n.convert<std::string>(str, out);

    protocol::WriteRedisValue(writer, out);
    writer->flush();

    EXPECT_STREQ("+abcd\r\n", writer->result.c_str());
}

//TEST(Marshaling, convert_exception) { TODO: Доделать тест
//
//}

TEST(Marshaling, compose_request) {
    std::shared_ptr<StringWriter> writer(new StringWriter(1024));

    std::string f_str = "f";
    auto n =  Marshaling<int>(writer, f_str, 1);
    n.compose_request(24);

    writer->flush();
    EXPECT_STREQ("+redis-rpc\r\n:1\r\n+id\r\n:24\r\n+method\r\n+f\r\n:1\r\n", writer->result.c_str());
}

TEST(Marshaling, compose_response) {
    std::shared_ptr<StringWriter> writer(new StringWriter(1024));

    std::string f_str = "2918273";
    auto n =  Marshaling<int>(writer, f_str);
    n.compose_response(24);

    writer->flush();
    EXPECT_STREQ("+redis-rpc\r\n:1\r\n+id\r\n:24\r\n+result\r\n+2918273\r\n", writer->result.c_str());
}

