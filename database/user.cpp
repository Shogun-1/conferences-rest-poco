#include "user.h"
#include "database.h"
#include "cache.h"
#include "../config/config.h"

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>
#include <cppkafka/cppkafka.h>

#include <sstream>
#include <exception>
#include <future>
#include <mutex>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

namespace database
{

    void User::init()
    {
        try
        {

            Poco::Data::Session session = database::Database::get().create_session();

            for (auto& hint : database::Database::get_all_hints())
            {
                Statement drop_stmt(session);
                drop_stmt << "DROP TABLE IF EXISTS `User`"
                          << hint,
                    now;

                Statement create_stmt(session);
                create_stmt << "CREATE TABLE IF NOT EXISTS `User` (`id` INT NOT NULL AUTO_INCREMENT,"
                            << "`login` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                            << "`password` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                            << "`first_name` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                            << "`last_name` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                            << "`email` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL,"
                            << "`title` VARCHAR(1024) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL,"
                            << "PRIMARY KEY (`id`), KEY `fn` (`first_name`), KEY `ln` (`last_name`))"
                            << hint,
                    now;
            }
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    Poco::JSON::Object::Ptr User::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("login", _login);
        root->set("password", _password);
        root->set("first_name", _first_name);
        root->set("last_name", _last_name);
        root->set("email", _email);
        root->set("title", _title);

        return root;
    }

    User User::fromJSON(const std::string &str)
    {
        User User;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        User.id() = object->getValue<long>("id");
        User.login() = object->getValue<std::string>("login");
        User.password() = object->getValue<std::string>("password");
        User.first_name() = object->getValue<std::string>("first_name");
        User.last_name() = object->getValue<std::string>("last_name");
        User.email() = object->getValue<std::string>("email");
        User.title() = object->getValue<std::string>("title");

        return User;
    }

    User User::read_by_id(long id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select(session);
            User a;
            select << "SELECT id, login, password, first_name, last_name, email, title FROM User where id=?",
                into(a._id),
                into(a._login),
                into(a._password),
                into(a._first_name),
                into(a._last_name),
                into(a._email),
                into(a._title),
                use(id),
                range(0, 1); //  iterate over result set one row at a time
  
            select.execute();
            Poco::Data::RecordSet rs(select);
            if (!rs.moveFirst()) throw std::logic_error("not found");

            return a;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    User User::read_by_login(std::string login)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select(session);
            User a;
            a._login = login;

            std::string shard = database::Database::sharding_hint(login);

            std::string select_op = "SELECT password, first_name, last_name, email, title FROM User where login=? ";
            select_op += shard;
            std::cout << select_op << std::endl;

            select << select_op,
                into(a._password),
                into(a._first_name),
                into(a._last_name),
                into(a._email),
                into(a._title),
                use(login),
                range(0, 1); //  iterate over result set one row at a time
  
            select.execute();
            Poco::Data::RecordSet rs(select);
            if (!rs.moveFirst()) throw std::logic_error("not found");

            return a;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    std::optional<User> User::read_from_cache_by_login(std::string login) {
        try
        {
            std::string result;
            if (database::Cache::get().get(login, result))
                return fromJSON(result);
            else
                return std::optional<User>();
        }
        catch (std::exception* err)
        {
            std::cerr << "error:" << err->what() << std::endl;
            throw;
        }
    }

    std::vector<User> User::read_all()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<User> result;
            User a;
            select << "SELECT id, login, password, first_name, last_name, email, title FROM User",
                into(a._id),
                into(a._login),
                into(a._password),
                into(a._first_name),
                into(a._last_name),
                into(a._email),
                into(a._title),
                range(0, 1); //  iterate over result set one row at a time

            while (!select.done())
            {
                if(select.execute())
                result.push_back(a);
            }
            return result;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    std::vector<User> User::search(std::string first_name, std::string last_name)
    {
        try
        {
            first_name = "\"" + first_name + "%\"";
            last_name = "\"" + last_name + "%\"";

            std::vector<User> result;
            std::vector<std::string> shards = database::Database::get_all_hints();
            std::vector<std::future<std::vector<User>>> futures;

            for (const std::string &shard : shards)
            {
                auto handle = std::async(std::launch::async, [first_name, last_name, shard]() -> std::vector<User>
                {
                    std::vector<User> result;

                    Poco::Data::Session session = database::Database::get().create_session();
                    Statement select(session);

                    std::string select_op = "SELECT login, first_name, last_name, email, title FROM User where first_name LIKE ";
                    select_op += first_name;
                    select_op += " AND last_name LIKE ";
                    select_op += last_name;
                    select_op += " ";
                    select_op += shard;

                    select << select_op,
                    select.execute();
                    Poco::Data::RecordSet record_set(select);

                    bool more = record_set.moveFirst();
                    while (more)
                    {
                        User user;

                        user._login = record_set[0].convert<std::string>();
                        user._first_name = record_set[1].convert<std::string>();
                        user._last_name = record_set[2].convert<std::string>();
                        user._email = record_set[3].convert<std::string>();
                        user._title = record_set[4].convert<std::string>();

                        result.push_back(user);

                        more = record_set.moveNext();
                    }

                    std::cout << select_op << std::endl;
                    
                    return result; 
                });

                futures.emplace_back(std::move(handle));
            }

            for (std::future<std::vector<User>> &res : futures)
            {
                std::vector<User> v = res.get();
                std::copy(std::begin(v), std::end(v), std::back_inserter(result));
            }

            return result;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    void User::send_to_queue(bool use_cache)
    {
        static cppkafka::Configuration config ={
            {"metadata.broker.list", Config::get().get_queue_host()},
            {"acks","all"}};
        static cppkafka::Producer producer(config);
        static std::mutex mtx;
        static int message_key{0};
        using Hdr = cppkafka::MessageBuilder::HeaderType;
        
        std::lock_guard<std::mutex> lock(mtx);
        std::stringstream ss;
        Poco::JSON::Stringifier::stringify(toJSON(), ss);
        std::string message = ss.str();
        bool not_sent = true;

        cppkafka::MessageBuilder builder(Config::get().get_queue_topic());
        std::string mk = std::to_string(++message_key);
        std::string use_cache_str = use_cache ? "true" : "false";

        builder.key(mk);
        builder.header(Hdr{"use_cache", use_cache_str});
        builder.payload(message);

        while (not_sent)
        {
            try
            {
                producer.produce(builder);
                not_sent = false;
            }
            catch (...) {}
        }
    }

    void User::save_to_mysql()
    {

        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement insert(session);

            std::string shard = database::Database::sharding_hint(_login);

            std::string insert_op = "INSERT INTO User (login, password, first_name, last_name, email, title) VALUES(?, ?, ?, ?, ?, ?) ";
            insert_op += shard;
            std::cout << insert_op << std::endl;

            insert << insert_op,
                use(_login),
                use(_password),
                use(_first_name),
                use(_last_name),
                use(_email),
                use(_title);

            insert.execute();

            Poco::Data::Statement select(session);
            select << "SELECT LAST_INSERT_ID()",
                into(_id),
                range(0, 1); //  iterate over result set one row at a time

            if (!select.done())
            {
                select.execute();
            }
            std::cout << "inserted:" << _id << std::endl;
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    void User::save_to_cache() {
        std::stringstream ss;
        Poco::JSON::Stringifier::stringify(toJSON(), ss);
        std::string message = ss.str();
        database::Cache::get().put(_login, message);
    }

    long User::get_id() const
    {
        return _id;
    }

    const std::string &User::get_login() const
    {
        return _login;
    }

    const std::string &User::get_password() const
    {
        return _password;
    }

    const std::string &User::get_first_name() const
    {
        return _first_name;
    }

    const std::string &User::get_last_name() const
    {
        return _last_name;
    }

    const std::string &User::get_email() const
    {
        return _email;
    }

    const std::string &User::get_title() const
    {
        return _title;
    }

    long &User::id()
    {
        return _id;
    }

    std::string &User::login()
    {
        return _login;
    }

    std::string &User::password()
    {
        return _password;
    }

    std::string &User::first_name()
    {
        return _first_name;
    }

    std::string &User::last_name()
    {
        return _last_name;
    }

    std::string &User::email()
    {
        return _email;
    }

    std::string &User::title()
    {
        return _title;
    }
}