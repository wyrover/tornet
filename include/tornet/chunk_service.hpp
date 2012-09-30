#ifndef _TN_CHUNK_SERVICE_HPP_
#define _TN_CHUNK_SERVICE_HPP_
#include <fc/shared_ptr.hpp>
#include <fc/fwd.hpp>
#include <fc/vector_fwd.hpp>
#include <tornet/tornet_file.hpp>
#include <tornet/link.hpp>

namespace fc {
  class path;
  class sha1;
  class ostream;
  class mutable_buffer;
}

namespace tn {

  namespace db {
    class chunk;
    class publish;
  }
  class node;
  class tornet_file;
  class chunk_service;
  class download_status;

  /**
   *  Determins if data is sufficiently random for the purposes of
   *  the chunk service.
   */
  bool      is_random( const fc::vector<char>& data );
  /**
   *  Takes arbitrary data and 'randomizes' it returning the MT19937 key
   *  that results in a random sequence that satisifies is_random()
   *
   *  Modifies data using the random sequence.
   */
  uint64_t  randomize( fc::vector<char>& data, uint64_t init_seed );

  /**
   *  Reverses the randomization performed by randomize
   *  @param seed - the value returned by randomize.
   */
  void      derandomize( uint64_t seed, fc::vector<char>& data );
  void      derandomize( uint64_t seed, const fc::mutable_buffer& b );

  /**
   *  Provides an interface to two chunk databases, one local and one cache.
   *  Handles requests from other nodes for chunks from the cache.
   *  Provides interface to upload and download chunks.
   *
   *  The local chunk database stores the chunks the user imports or downloads without
   *  respect to their distance from the node or access patterns. 
   *
   *  The cache chunk database stores chunks for other people so that it might earn
   *  credit with other nodes.  This cache database stores chunks based upon their
   *  access rate and distance from the node.
   *
   *  All chunks are random data encrypted via blowfish from the original file.  The blowfish
   *  key is the hash of the original file.  To restore a file you must know the hash of the
   *  file description chunk as well as the hash of the resulting file.  
   */
  class chunk_service : virtual public fc::retainable {
    public:
      typedef fc::shared_ptr<chunk_service> ptr;

      chunk_service( const fc::path&      dbdir, const fc::shared_ptr<tn::node>& n );
      virtual ~chunk_service();

      void shutdown();
       
      fc::shared_ptr<tn::db::chunk>&    get_cache_db();
      fc::shared_ptr<tn::db::chunk>&    get_local_db();
      fc::shared_ptr<tn::db::publish>&  get_publish_db();
      fc::shared_ptr<tn::node>&         get_node();

      /**
       *  Loads the file from disk, turns it into chunks and then
       *  starts publishing those chunks on the network at the
       *  desired replication rate.
       *
       *  All published files show up in the link database. Once
       *  the file has been published, local changes are not reflected,
       *  it will have to be removed from the link DB.
       */
      tn::link publish( const fc::path& file, uint32_t rep = 3 );
     
      /**
       *  Reads the data for the chunk from the cache or local database.
       *
       *  @return an empty vector if no chunk was found
       */
      fc::vector<char> fetch_chunk( const fc::sha1& chunk_id );

      /**
       *  Assuming the data is stored locally, returns a tornet file,
       *  otherwise throws an exception if the chunk has not been 
       *  downloaded.
       */
      tornet_file fetch_tornet( const tn::link& ln );

      /**
       *  Performs a synchronous download of the specified chunk
       */
      fc::vector<char> download_chunk( const fc::sha1& chunk_id );

      /**
       *  Effectively the same as download_chunk(); fetch_tornet();
       */
      tornet_file download_tornet( const tn::link& ln );

      void enable_publishing( bool state );
      bool is_publishing()const;
    private:
      class impl;
      fc::fwd<impl,408> my;
  };

}


#endif // _CHUNK_SERVICE_HPP_
