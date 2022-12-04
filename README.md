# Guidelines  
## Setting up a database and filling it with data
1. Please type the following command in bash to create a database and fill it with the test data:  
`mysql <your_db_name> < commands.sql -p`  
For example, you can use `conf_db` name for your database. In this case, you won't need to update the launcher script afterwards.  
Be careful, before creating new tables old ones will be deleted if there are any.
2. Then type the password.
3. After a couple of seconds all the necessary tables will be created, then the data will be loaded into the database. 
4. The database is ready!

##  Starting a web-server for serving REST requests
Please launch [./start.sh](start.sh) script.  
Don't forget to update the database name in the script if it's needed (by default `conf_db` database is used) and MYSQL login credentials.

## OpenAPI specification
You can check OpenAPI specification in [index.yaml](index.yaml) file. 

# Sharding functionality
1. Use `docker-compose up --build` to run two MySQL database containers and one ProxySQL container.
2. Run `./start.sh` to start the web-server.
3. Feel free to use [index.yaml](index.yaml) to test requests. User entries should be equally split between two database nodes based on the user login`s hash key.

# Caching
As of 04/12/2022 caching is supported. All `User` entries will be cached now by default for increased performance. Redis is used for caching.  
1. You can set `no_cache` parameter in a POST query to `true` in order not to use caching for some particular entry while saving it to the database.
2. You can set `no_cache` parameter in a GET query to `true` in order not to cache the entry (if it hasn't already been cached) while fetching it from the database. Cache won't be used at all, the entry will be fetched directly from the database. Supported only in the search queries when the user's login is given.  
3. It is recommended to set `no_cache` parameter to `false` everytime by default to use caching.  
---  
