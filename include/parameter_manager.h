/*    This file is part of parameter manager
      The aim of this software is to provide basic facilities to handle
      command line parameters
      Copyright (C) 2013  Julien Thevenon ( julien_thevenon at yahoo.fr )

      This program is free software: you can redistribute it and/or modify
      it under the terms of the GNU General Public License as published by
      the Free Software Foundation, either version 3 of the License, or
      (at your option) any later version.

      This program is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
      GNU General Public License for more details.

      You should have received a copy of the GNU General Public License
      along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef PARAMETER_MANAGER_H
#define PARAMETER_MANAGER_H

#include "parameter_if.h"
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>

namespace parameter_manager
{
  class parameter_manager
  {
  public:
    parameter_manager(const std::string & p_name,
		      const std::string p_prefix,
		      uint32_t p_nb_mandatory);
    void add(parameter_if & p_parameter);
    void treat_parameters(int argc, char ** argv);
    void display(void)const;
  private:
    uint32_t m_nb_mandatory;
    const std::string m_name;
    const std::string m_prefix;
    std::map<std::string,parameter_if*> m_prefixed_parameters;
    std::vector<parameter_if*> m_ordered_parameters;
    std::map<std::string,parameter_if*> m_all_parameters;
  };

  //----------------------------------------------------------------------------
  parameter_manager::parameter_manager(const std::string & p_name,
				       const std::string p_prefix,
				       uint32_t p_nb_mandatory):
    m_nb_mandatory(p_nb_mandatory),
    m_name(p_name),
    m_prefix(p_prefix)
      {
      }

    //----------------------------------------------------------------------------
    void parameter_manager::add(parameter_if & p_parameter)
    {
      std::string l_name = p_parameter.get_name();
      std::map<std::string,parameter_if*>::const_iterator l_iter = m_all_parameters.find(l_name);
      if(l_iter != m_all_parameters.end())
	{
	  std::cout << "ERROR : a parameter named \"" << l_name << "\" was already defined" << std::endl ;
	  exit(-1);
	}
      m_all_parameters.insert(std::map<std::string,parameter_if*>::value_type(l_name,&p_parameter));
      if(p_parameter.is_prefixed())
	{
	  m_prefixed_parameters.insert(std::map<std::string,parameter_if*>::value_type(l_name,&p_parameter));
	}
      else
	{
	  m_ordered_parameters.push_back(&p_parameter);
	}
    }

    //----------------------------------------------------------------------------
    void parameter_manager::treat_parameters(int argc, char ** argv)
    {
      assert(m_nb_mandatory <= m_ordered_parameters.size());
      uint32_t l_nb_ordered_parameters_encountered = 0;
      for(int32_t l_index = 1;l_index< argc;++l_index)
	{
#ifdef DEBUG_PARAM_MANAGER
	  std::cout << "Treating params[" << l_index << "]=\"" << argv[l_index] << "\"" << std::endl ;
#endif
	  parameter_if *l_parameter = NULL;
	  std::string l_value;

	  std::string l_arg(argv[l_index]);

	  // Check if parameter is prefixed or not
	  if(l_arg.size() >= m_prefix.length() && l_arg.substr(0,m_prefix.length())==m_prefix)
	    {
	      // Removing prefix
	      l_arg = l_arg.substr(m_prefix.length());

	      // Searching assignation operator to be able to extract param name and value
	      std::size_t l_pos = l_arg.find("=");
	      if(l_pos == std::string::npos)
		{
		  std::cout << "ERROR : bad syntax in parameter assignment. It should be " << m_prefix << "<param_name>=<param_value>" << std::endl ;
		  display();
		  exit(-1);
		}

	      // Get param name
	      std::string l_param_name = l_arg.substr(0,l_pos);

	      // Checking if param is defined
	      std::map<std::string,parameter_if*>::const_iterator l_iter = m_prefixed_parameters.find(l_param_name);
	      if(l_iter == m_prefixed_parameters.end())
		{
		  std::cout << "ERROR : undefined parameter \"" << l_param_name << "\"" << std::endl ;
		  display();
		  exit(-1);
		}

	      // Preparing assignation
	      l_parameter = l_iter->second;
	      l_value = l_arg.substr(l_pos+1);
	    }
	  else
	    {
	      if(l_nb_ordered_parameters_encountered >= m_ordered_parameters.size())
		{
		  std::cout << "Too many unprefixed parameters : \"" << l_arg << "\"" << std::endl ;
		  display();
		  exit(-1);		  
		}
	      l_parameter = m_ordered_parameters[l_nb_ordered_parameters_encountered];
	      l_value = l_arg;
	      ++l_nb_ordered_parameters_encountered;
	    }

	  std::cout << "Assigning value \"" << l_value << "\" to parameter \"" << l_parameter->get_name() << "\"" << std::endl;
	  l_parameter->set_text_value(l_value);
	}

      if(l_nb_ordered_parameters_encountered < m_nb_mandatory)
	{
	  std::cout << "ERROR : the following mandatory parameters are missing : " << std::endl ;
	  for(uint32_t l_index = l_nb_ordered_parameters_encountered ; l_index < m_nb_mandatory ; ++l_index)
	    {
	      std::cout << m_ordered_parameters[l_index]->get_name() << std::endl ;
	    }
	  display();
	  exit(1);
	}
    }

    //----------------------------------------------------------------------------
    void parameter_manager::display(void)const
    {
      std::cout << "Usage is :" << std::endl ;
      std::cout << "\t" << m_name ;
      if(m_prefixed_parameters.size())
	{
	  std::cout << " [OPTIONS]" ;
	}
      std::vector<parameter_if*>::const_iterator l_iter_ordered = m_ordered_parameters.begin();
      std::vector<parameter_if*>::const_iterator l_iter_end_ordered = m_ordered_parameters.end();
      uint32_t l_parameter_index = 0;
      while(l_iter_ordered != l_iter_end_ordered)
	{
	  std::cout << " " ;
	  if(l_parameter_index >= m_nb_mandatory)
	    {
	      std::cout <<"[" ;
	    }
	  std::cout << "<" << (*l_iter_ordered)->get_name() << ">" ;
	  if(l_parameter_index >= m_nb_mandatory)
	    {
	      std::cout <<"]" ;
	    }
	  ++l_parameter_index;
	  ++l_iter_ordered;
	}
      std::cout << std::endl ;
      if(m_prefixed_parameters.size())
	{
	  std::cout << "OPTIONS : " << m_prefix << "<paremeter_name>=<parameter_value>" << std::endl ;
	  std::map<std::string,parameter_if*>::const_iterator l_iter = m_prefixed_parameters.begin();
	  std::map<std::string,parameter_if*>::const_iterator l_iter_end = m_prefixed_parameters.end();
	  while(l_iter != l_iter_end)
	    {
	      std::cout << "\t" << m_prefix << l_iter->first << "=..." << std::endl ;
	      ++l_iter;
	    }
	}
    }
}
#endif // PARAMETER_MANAGER_H
//EOF
