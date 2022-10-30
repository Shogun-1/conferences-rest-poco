#ifndef CONF_H
#define CONF_H

#include <string>
#include <vector>
#include "Poco/JSON/Object.h"

#include "pres.h"

namespace database
{
    class Conf{
        private:
            long _id;
            std::string _title;
            std::string _starttime;
            std::string _place;

        public:

            static Conf fromJSON(const std::string & str);

            long             get_id() const;
            const std::string &get_title() const;
            const std::string &get_starttime() const;
            const std::string &get_place() const;

            long&        id();
            std::string &title();
            std::string &starttime();
            std::string &place();

            static void init();
            static Conf get(std::string title);
            static long add_pres(std::string conf_title, std::string pres_title);
            static std::vector<Pres> get_pres_list(std::string conf_title);
            void save_to_mysql();

            Poco::JSON::Object::Ptr toJSON() const;

    };
}

#endif