#include "conf.h"
#include "pres.h"
#include "database.h"
#include "../config/config.h"

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

#include <sstream>
#include <exception>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

namespace database
{

    void Conf::init()
    {
        try
        {

            Poco::Data::Session session = database::Database::get().create_session();
            Statement drop_stmt(session);
            drop_stmt << "DROP TABLE IF EXISTS `Conference`", now;

            Statement create_stmt(session);
            create_stmt << "CREATE TABLE IF NOT EXISTS `Conference` (`id` INT NOT NULL AUTO_INCREMENT,"
                        << "`title` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                        << "`starttime` DATETIME NOT NULL,"
                        << "`place` VARCHAR(512) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                        << "PRIMARY KEY (`id`), KEY (`title`));",
                now;

            Statement drop_stmt2(session);
            drop_stmt2 << "DROP TABLE IF EXISTS `Conf_Pres`", now;

            Statement create_stmt2(session);
            create_stmt2 << "CREATE TABLE IF NOT EXISTS `Conf_Pres` (`id` INT NOT NULL AUTO_INCREMENT,"
                        << "`conf_id` INT NOT NULL,"
                        << "`pres_id` INT NOT NULL,"
                        << "`conf_title` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                        << "`pres_title` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                        << "PRIMARY KEY (`id`),"
                        << "FOREIGN KEY (`conf_id`) REFERENCES `Conference`(`id`) ON UPDATE CASCADE ON DELETE CASCADE,"
                        << "FOREIGN KEY (`pres_id`) REFERENCES `Presentation`(`id`) ON UPDATE CASCADE ON DELETE CASCADE);",
                now;
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

    Poco::JSON::Object::Ptr Conf::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("title", _title);
        root->set("starttime", _starttime);
        root->set("place", _place);

        return root;
    }

    Conf Conf::fromJSON(const std::string &str)
    {
        Conf conf;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        conf.id() = object->getValue<long>("id");
        conf.title() = object->getValue<std::string>("title");
        conf.starttime() = object->getValue<std::string>("starttime");
        conf.place() = object->getValue<std::string>("place");

        return conf;
    }

    Conf Conf::get(std::string title)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            Conf conf;
            select << "SELECT id, title, CAST(starttime AS CHAR), place FROM Conference WHERE title=?",
                into(conf._id),
                into(conf._title),
                into(conf._starttime),
                into(conf._place),
                use(title),
                range(0, 1); //  iterate over result set one row at a time

            select.execute();
            Poco::Data::RecordSet rs(select);
            if (!rs.moveFirst()) 
            {
                throw std::logic_error("not found");
            }
            return conf;
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

    long Conf::add_pres(std::string conf_title, std::string pres_title)
    {
        try
        {
            Conf conf = Conf::get(conf_title);
            Pres pres = Pres::get(pres_title);

            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement insert(session);

            insert << "INSERT INTO Conf_Pres (conf_id, pres_id, conf_title, pres_title) VALUES(?, ?, ?, ?)",
                use(conf.id()),
                use(pres.id()),
                use(conf.title()),
                use(pres.title()),

            insert.execute();

            Poco::Data::Statement select(session);
            long insert_id = 0;
            select << "SELECT LAST_INSERT_ID()",
                into(insert_id),
                range(0, 1); //  iterate over result set one row at a time

            if (!select.done())
            {
                select.execute();
            }
            std::cout << "inserted:" << insert_id << std::endl;
            return insert_id;
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

    std::vector<Pres> Conf::get_pres_list(std::string conf_title)
    {
        try
        {
            Conf request_conf = Conf::get(conf_title);

            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<Pres> result;
            Pres pres;
            select << "SELECT pres_title FROM Conf_Pres WHERE conf_id=?",
                into(pres.title()),
                use(request_conf.id()),
                range(0, 1); //  iterate over result set one row at a time

            while (!select.done())
            {
                if(select.execute())
                result.push_back(pres);
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
   
    void Conf::save_to_mysql()
    {

        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement insert(session);

            insert << "INSERT INTO Conference(title, starttime, place) VALUES(?, ?, ?)",
                use(_title),
                use(_starttime),
                use(_place),

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

    long Conf::get_id() const
    {
        return _id;
    }

    const std::string &Conf::get_title() const
    {
        return _title;
    }

    const std::string &Conf::get_starttime() const
    {
        return _starttime;
    }

    const std::string &Conf::get_place() const
    {
        return _place;
    }

    long &Conf::id()
    {
        return _id;
    }

    std::string &Conf::title()
    {
        return _title;
    }

    std::string &Conf::starttime()
    {
        return _starttime;
    }

    std::string &Conf::place()
    {
        return _place;
    }

}