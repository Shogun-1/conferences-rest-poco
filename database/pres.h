#ifndef PRES_H
#define PRES_H

#include <string>
#include <vector>
#include "Poco/JSON/Object.h"

namespace database
{
    class Pres{
        private:
            long _id;
            std::string _title;
            std::string _theme;
            std::string _annotation;
            std::string _author;
            std::string _date;

        public:

            static Pres fromJSON(const std::string & str);

            long             get_id() const;
            const std::string &get_title() const;
            const std::string &get_theme() const;
            const std::string &get_annotation() const;
            const std::string &get_author() const;
            const std::string &get_date() const;

            long&        id();
            std::string &title();
            std::string &theme();
            std::string &annotation();
            std::string &author();
            std::string &date();

            static void init();
            static std::vector<Pres> get_all();
            static Pres get(std::string title);
            void save_to_mysql();

            Poco::JSON::Object::Ptr toJSON() const;

    };
}

#endif