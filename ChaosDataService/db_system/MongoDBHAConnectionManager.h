/*
 *	MongoDBHAConnection.h
 *	!CHOAS
 *	Created by Bisegni Claudio.
 *
 *    	Copyright 2014 INFN, National Institute of Nuclear Physics
 *
 *    	Licensed under the Apache License, Version 2.0 (the "License");
 *    	you may not use this file except in compliance with the License.
 *    	You may obtain a copy of the License at
 *
 *    	http://www.apache.org/licenses/LICENSE-2.0
 *
 *    	Unless required by applicable law or agreed to in writing, software
 *    	distributed under the License is distributed on an "AS IS" BASIS,
 *    	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    	See the License for the specific language governing permissions and
 *    	limitations under the License.
 */

#ifndef __CHAOSFramework__MongoDBHAConnection__
#define __CHAOSFramework__MongoDBHAConnection__

#include "mongo/client/dbclient.h"

#include <vector>
#include <string>
#include <queue>
#include <map>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <chaos/common/data/CDataWrapper.h>

namespace chaos_data = chaos::common::data;

namespace chaos {
	namespace data_service {
		namespace db_system {
			
			/*!
			 Class that encapsulat ethe mongodb conenction for safe deallocation
			 */
			class DriverScopedConnection : public mongo::ScopedDbConnection {
				
			public:
				DriverScopedConnection(mongo::ConnectionString _conn);
				~DriverScopedConnection();
			};
			
			typedef DriverScopedConnection *MongoDBHAConnection;
			
			class MongoAuthHook : public mongo::DBConnectionHook {
				std::string user;
				std::string pwd;
				std::string db;
				bool has_autentication;
				void onCreate( mongo::DBClientBase * conn );
				void onHandedOut( mongo::DBClientBase * conn );
				void onDestroy( mongo::DBClientBase * conn );
			public:
				MongoAuthHook(std::map<string,string>& key_value_custom_param);
			};
			
			/*!
			 Implementation for the high availability with multiple
			 mongos router instance
			 */
			class MongoDBHAConnectionManager {
				uint32_t server_number;
				
				uint64_t next_retrive_intervall;
				
				boost::shared_mutex mutext_queue;
				std::queue< boost::shared_ptr<mongo::ConnectionString> > valid_connection_queue;
				std::queue< boost::shared_ptr<mongo::ConnectionString> > offline_connection_queue;
				
				inline bool canRetry();
				
				bool getConnection(MongoDBHAConnection *connection_sptr);
				
			public:
				MongoDBHAConnectionManager(std::vector<std::string> monogs_routers_list, std::map<string,string>& key_value_custom_param);
				~MongoDBHAConnectionManager();
				
				int insert( const std::string &ns , mongo::BSONObj obj , int flags=0);
				int findOne( mongo::BSONObj& result, const std::string &ns, const mongo::Query& query, const mongo::BSONObj *fieldsToReturn = 0, int queryOptions = 0);
				void findN(std::vector<mongo::BSONObj>& out, const std::string& ns, mongo::Query query, int nToReturn, int nToSkip = 0, const mongo::BSONObj *fieldsToReturn = 0, int queryOptions = 0);
				int runCommand( mongo::BSONObj& result, const std::string &ns, const mongo::BSONObj& comand, int queryOptions = 0);
				int update( const std::string &ns, mongo::Query query, mongo::BSONObj obj, bool upsert = false, bool multi = false, const mongo::WriteConcern* wc=NULL );
				int remove( const std::string &ns , mongo::Query q , bool justOne = 0, const mongo::WriteConcern* wc=NULL );
				int ensureIndex(  const std::string &database, const std::string &collection, mongo::BSONObj keys, bool unique = false, const std::string &name = "", bool dropDup = false, bool background = false, int v = -1, int ttl = 0 );
				std::auto_ptr<mongo::DBClientCursor> query(const std::string &ns, mongo::Query query, int nToReturn = 0, int nToSkip = 0, const mongo::BSONObj *fieldsToReturn = 0, int queryOptions = 0 , int batchSize = 0 );
				int count(unsigned long long & result, const std::string &ns, const mongo::Query& query = mongo::Query(), int options=0, int limit=0, int skip=0 );
			};
		}
	}
}

#endif /* defined(__CHAOSFramework__MongoDBHAConnection__) */
