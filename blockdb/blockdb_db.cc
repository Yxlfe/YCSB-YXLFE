//
// Created by wujy on 18-1-21.
//


#include <iostream>

#include "blockdb/blockdb_db.h"
#include "lib/coding.h"


using namespace std;

namespace ycsbc {
    LevelDB::LevelDB(const char *dbfilename, utils::Properties &props) :noResult(0){
        
        //set option
        leveldb::Options options;
        SetOptions(&options, props);
        std::cout << "20 options: " << options.create_if_missing <<endl;

// #ifndef NOVELSM
        leveldb::Status s = leveldb::DB::Open(options, dbfilename, &db_);

// #else   //novelsm
//         std::string db_mem = "/pmem/nvm";
//         leveldb::Status s = leveldb::DB::Open(options,dbfilename, db_mem, &db_);  //novelsm
// #endif
        if(!s.ok()){
            cerr<<"Can't open leveldb "<<dbfilename<<" "<<s.ToString()<<endl;
            exit(0);
        }
    }
    void LevelDB::SetOptions(leveldb::Options *options, utils::Properties &props) {

        //// 默认的Leveldb配置
        options->create_if_missing = true;
        options->compression = leveldb::kNoCompression;
        options->write_buffer_size = (16 << 20);
        options->compaction = leveldb::kBlockCompaction;
        options->filter_policy = leveldb::NewBloomFilterPolicy(10);
        options->level0_slowdown_writes_trigger = 12;
        options->level0_stop_writes_trigger = 16;
        options->block_cache = leveldb::NewLRUCache(4LL << 30);
        options->direct_io = true;
        options->max_file_size = (16 << 20);
        ////

//         int dboption = stoi(props["dboption"]);

//         if ( dboption == 1) {  //Novelsm options
// #ifdef NOVELSM 
//             printf("set Novelsm options!\n");
//             //options->db_mem = "/pmem/nvm";
//             options->write_buffer_size = 64ul * 1024 * 1024;
//             options->nvm_buffer_size = 4096ul * 1024 * 1024;
//             options->num_levels = 2;
//             // /options->cnsdl = 0;
// #endif
//         }
        
    }

    int LevelDB::Read(const std::string &table, const std::string &key, const std::vector<std::string> *fields,
                      std::vector<KVPair> &result) {
        string value;
        leveldb::Status s = db_->Get(leveldb::ReadOptions(),key,&value);
        //printf("read:key:%lu-%s [%lu]\n",key.size(),key.data(),value.size());
        if(s.ok()) {
            //printf("value:%lu\n",value.size());
            DeSerializeValues(value, result);
            /* printf("get:key:%lu-%s\n",key.size(),key.data());
            for( auto kv : result) {
                printf("get field:key:%lu-%s value:%lu-%s\n",kv.first.size(),kv.first.data(),kv.second.size(),kv.second.data());
            } */
            return DB::kOK;
        }
        if(s.IsNotFound()){
            noResult++;
            //cerr<<"read not found:"<<noResult<<endl;
            return DB::kOK;
        }else{
            cerr<<"read error"<<endl;
            exit(0);
        }
    }

    int LevelDB::Scan(const std::string &table, const std::string &key, const std::string &max_key, int len, const std::vector<std::string> *fields,
                      std::vector<std::vector<KVPair>> &result) {
        auto it=db_->NewIterator(leveldb::ReadOptions());
        it->Seek(key);
        std::string val;
        std::string k;
        for(int i=0;i<len && it->Valid();i++){
            k = it->key().ToString();
            val = it->value().ToString();
            it->Next();
        }
        delete it;
        return DB::kOK;
    }

    int LevelDB::Insert(const std::string &table, const std::string &key,
               std::vector<KVPair> &values){
        leveldb::Status s;
        string value;
        SerializeValues(values,value);
        //printf("put:key:%lu-%s [%lu]\n",key.size(),key.data(),value.size());
        /* for( auto kv : values) {
            printf("put field:key:%lu-%s value:%lu-%s\n",kv.first.size(),kv.first.data(),kv.second.size(),kv.second.data());
        } */ 
        
        s = db_->Put(leveldb::WriteOptions(), key, value);
        if(!s.ok()){
            cerr<<"insert error\n"<<endl;
            exit(0);
        }
        
        return DB::kOK;
    }

    int LevelDB::Update(const std::string &table, const std::string &key, std::vector<KVPair> &values) {
        return Insert(table,key,values);
    }

    int LevelDB::Delete(const std::string &table, const std::string &key) {
        leveldb::Status s;
        s = db_->Delete(leveldb::WriteOptions(),key);
        if(!s.ok()){
            cerr<<"Delete error\n"<<endl;
            exit(0);
        }
        return DB::kOK;
    }

    void LevelDB::PrintStats() {
        cout<<"read not found:"<<noResult<<endl;
        string stats;
        db_->GetProperty("leveldb.stats",&stats);
        cout<<stats<<endl;
    }

    // bool LevelDB::HaveBalancedDistribution() {
    //     return db_->HaveBalancedDistribution();
    // }

    LevelDB::~LevelDB() {
        delete db_;
    }

    void LevelDB::SerializeValues(std::vector<KVPair> &kvs, std::string &value) {
        value.clear();
        PutFixed64(&value, kvs.size());
        for(unsigned int i=0; i < kvs.size(); i++){
            //kvs[i].first = field*...
            //PutFixed64:8bytes fill size
            PutFixed64(&value, kvs[i].first.size());
            value.append(kvs[i].first);
            //kvs[i].second = rand ASCII(len = fieldlength)
            PutFixed64(&value, kvs[i].second.size());
            value.append(kvs[i].second);
        }
    }

    void LevelDB::DeSerializeValues(std::string &value, std::vector<KVPair> &kvs){
        uint64_t offset = 0;
        uint64_t kv_num = 0;
        uint64_t key_size = 0;
        uint64_t value_size = 0;

        kv_num = DecodeFixed64(value.c_str());
        offset += 8;
        for( unsigned int i = 0; i < kv_num; i++){
            ycsbc::DB::KVPair pair;
            key_size = DecodeFixed64(value.c_str() + offset);
            offset += 8;

            pair.first.assign(value.c_str() + offset, key_size);
            offset += key_size;

            value_size = DecodeFixed64(value.c_str() + offset);
            offset += 8;

            pair.second.assign(value.c_str() + offset, value_size);
            offset += value_size;
            kvs.push_back(pair);
        }
    }
}
