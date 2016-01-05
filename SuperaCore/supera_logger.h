/**
 * \file supera_logger.h
 *
 * \ingroup SupareCore
 * 
 * \brief logger utility class definition header file.
 *
 * @author Kazu - Nevis 2015
 */

/** \addtogroup SuperaCore

    @{*/


#ifndef __SUPERALOGGER_H__
#define __SUPERALOGGER_H__

#include <cstdio>
#include <iostream>
#include <map>
#include "SuperaTypes.h"

namespace larcaffe {

  namespace supera {
    /**
       \class logger
       \brief Utility class used to show formatted message on the screen.
    */
    class logger{
      
    public:
      
      /// Default constructor
      logger(const std::string& name="no_name")
	: _ostrm(&std::cout)
	, _name(name)
      {}
      
      /// Default destructor
      virtual ~logger(){};

    private:
      
      /// ostream
      std::ostream *_ostrm;
      
      /// Level
      msg::Level_t _level;
      
      /// Name
      std::string _name;

      /// Set of loggers
      static std::map<std::string,larcaffe::supera::logger> *_logger_m;
      
    public:
      
      const std::string& name() const { return _name; }
      
      void set(const msg::Level_t level) { _level = level; }
      
      msg::Level_t level() const { return _level; }
      
      inline bool operator<(const logger& rhs) const
      {
	if(_name < rhs.name()) return true;
	if(_name > rhs.name()) return false;
	return false;
      }
      
      /// Getter of a message instance 
      static logger& get(const std::string name)
      {
	if(!_logger_m) _logger_m = new std::map<std::string,larcaffe::supera::logger>();
	auto iter = _logger_m->find(name);
	if(iter == _logger_m->end())
	  iter = _logger_m->emplace(name,logger(name)).first;
	return iter->second;
      };
      
      inline bool debug   () const { return _level <= msg::kDEBUG;   }
      inline bool info    () const { return _level <= msg::kINFO;    }
      inline bool normal  () const { return _level <= msg::kNORMAL;  }
      inline bool warning () const { return _level <= msg::kWARNING; }
      inline bool error   () const { return _level <= msg::kERROR;   }
      
      std::ostream& LOG(const msg::Level_t) const;
      
      std::ostream& LOG(const msg::Level_t level,
			const std::string& function ) const;

      std::ostream& LOG(const msg::Level_t level,
			const std::string& function,
			const unsigned int line_num ) const;
      
      std::ostream& LOG(const msg::Level_t level,
			const std::string& function,
			const unsigned int line_num,
			const std::string& file_name) const;
      
    };
  }
}
#endif
  
/** @} */ // end of doxygen group logger
