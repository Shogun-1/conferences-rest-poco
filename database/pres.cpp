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

    void Pres::init()
    {
        try
        {

            Poco::Data::Session session = database::Database::get().create_session();
            Statement drop_stmt(session);
            drop_stmt << "DROP TABLE IF EXISTS `Presentation`", now;

            Statement create_stmt(session);
            create_stmt << "CREATE TABLE IF NOT EXISTS `Presentation` (`id` INT NOT NULL AUTO_INCREMENT,"
                        << "`title` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                        << "`theme` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                        << "`annotation` TEXT CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL,"
                        << "`author` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                        << "`date` DATE NOT NULL,"
                        << "PRIMARY KEY (`id`));",
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

    Poco::JSON::Object::Ptr Pres::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("title", _title);
        root->set("theme", _theme);
        root->set("annotation", _annotation);
        root->set("author", _author);
        root->set("date", _date);

        return root;
    }

    Pres Pres::fromJSON(const std::string &str)
    {
        Pres pres;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        pres.id() = object->getValue<long>("id");
        pres.title() = object->getValue<std::string>("title");
        pres.theme() = object->getValue<std::string>("theme");
        pres.annotation() = object->getValue<std::string>("annotation");
        pres.author() = object->getValue<std::string>("author");
        pres.date() = object->getValue<std::string>("date");

        return pres;
    }

    std::vector<Pres> Pres::get_all()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<Pres> result;
            Pres pres;
            select << "SELECT title, theme, annotation, author, CAST(date AS CHAR) FROM Presentation",
                into(pres._title),
                into(pres._theme),
                into(pres._annotation),
                into(pres._author),
                into(pres._date),
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
   
    void Pres::save_to_mysql()
    {

        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement insert(session);

            insert << "INSERT INTO Presentation (title, theme, annotation, author, date) VALUES(?, ?, ?, ?, ?)",
                use(_title),
                use(_theme),
                use(_annotation),
                use(_author),
                use(_date),

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

    long Pres::get_id() const
    {
        return _id;
    }

    const std::string &Pres::get_title() const
    {
        return _title;
    }

    const std::string &Pres::get_theme() const
    {
        return _theme;
    }

    const std::string &Pres::get_annotation() const
    {
        return _annotation;
    }

    const std::string &Pres::get_author() const
    {
        return _author;
    }

    const std::string &Pres::get_date() const
    {
        return _date;
    }

    long &Pres::id()
    {
        return _id;
    }

    std::string &Pres::title()
    {
        return _title;
    }

    std::string &Pres::theme()
    {
        return _theme;
    }

    std::string &Pres::annotation()
    {
        return _annotation;
    }

    std::string &Pres::author()
    {
        return _author;
    }

    std::string &Pres::date()
    {
        return _date;
    }

}