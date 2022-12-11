#ifndef USER_H
#define USER_H

#include <string>
#include <vector>
#include "Poco/JSON/Object.h"

namespace database
{
    class User{
        private:
            long _id;
            std::string _login;
            std::string _password;
            std::string _first_name;
            std::string _last_name;
            std::string _email;
            std::string _title;

        public:

            static User fromJSON(const std::string & str);

            long             get_id() const;
            const std::string &get_login() const;
            const std::string &get_password() const;
            const std::string &get_first_name() const;
            const std::string &get_last_name() const;
            const std::string &get_email() const;
            const std::string &get_title() const;

            long&        id();
            std::string &login();
            std::string &password();
            std::string &first_name();
            std::string &last_name();
            std::string &email();
            std::string &title();

            static void init();
            static User read_by_id(long id);
            static User read_by_login(std::string login);
            static std::optional<User> read_from_cache_by_login(std::string login);
            static std::vector<User> read_all();
            static std::vector<User> search(std::string first_name,std::string last_name);
            void save_to_mysql();
            void save_to_cache();
            void send_to_queue(bool use_cache);

            Poco::JSON::Object::Ptr toJSON() const;

    };
}

#endif