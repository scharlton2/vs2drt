// Surface.cxx: implementation of the cxxSurface class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#pragma warning(disable : 4786)	// disable truncation warning (Only used by debugger)
#endif
#include <cassert>				// assert
#include <algorithm>			// std::sort
#include "Phreeqc_class.h"
#include "Utils.h"				// define first
#if !defined(PHREEQC_CLASS)
#define EXTERNAL extern
#include "global.h"
#else
#include "Phreeqc.h"
#endif
#include "Surface.h"
#include "cxxMix.h"
#include "phqalloc.h"
#include "phrqproto.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

cxxSurface::cxxSurface()
	//
	// default constructor for cxxSurface 
	//
:	cxxNumKeyword()
{
	type = DDL;
	dl_type = NO_DL;
	sites_units = SITES_ABSOLUTE;
	//diffuse_layer           = false;
	//edl                     = false;
	only_counter_ions = false;
	//donnan                  = false;
	thickness = 1e-8;
	debye_lengths = 0.0;
	DDL_viscosity = 1.0;
	DDL_limit = 0.8;
	transport = false;
}

cxxSurface::cxxSurface(struct surface *surface_ptr)
		//
		// constructor for cxxSurface from struct surface
		//
	:
cxxNumKeyword()
{
	int i;

	this->set_description(surface_ptr->description);
	n_user = surface_ptr->n_user;
	n_user_end = surface_ptr->n_user_end;
	type = surface_ptr->type;
	dl_type = surface_ptr->dl_type;
	sites_units = surface_ptr->sites_units;
	//diffuse_layer                = (surface_ptr->diffuse_layer == TRUE); 
	//edl                          = (surface_ptr->edl == TRUE); 
	only_counter_ions = (surface_ptr->only_counter_ions == TRUE);
	//donnan                       = (surface_ptr->donnan == TRUE); 
	thickness = surface_ptr->thickness;
	debye_lengths = surface_ptr->debye_lengths;
	DDL_viscosity = surface_ptr->DDL_viscosity;
	DDL_limit = surface_ptr->DDL_limit;
	transport = (surface_ptr->transport == TRUE);
	// Surface components
	for (i = 0; i < surface_ptr->count_comps; i++)
	{
		cxxSurfaceComp ec(&(surface_ptr->comps[i]));
		std::string str(ec.get_formula());
		surfaceComps[str] = ec;
	}
	// Surface charge
	for (i = 0; i < surface_ptr->count_charge; i++)
	{
		cxxSurfaceCharge ec(&(surface_ptr->charge[i]));
		std::string str(ec.get_name());
		surfaceCharges[str] = ec;
	}
}
cxxSurface::cxxSurface(PHREEQC_PTR_ARG_COMMA const std::map < int, cxxSurface > &entities,
					   cxxMix & mix, int l_n_user):
cxxNumKeyword()
{
	this->n_user = this->n_user_end = l_n_user;
	type = DDL;
	dl_type = NO_DL;
	sites_units = SITES_ABSOLUTE;
	only_counter_ions = false;
	thickness = 1e-8;
	debye_lengths = 0.0;
	DDL_viscosity = 1.0;
	DDL_limit = 0.8;
	transport = false;
//
//   Mix exchangers
//
	std::map < int, double >*mixcomps = mix.comps();
	std::map < int, double >::const_iterator it;
	for (it = mixcomps->begin(); it != mixcomps->end(); it++)
	{
		if (entities.find(it->first) != entities.end())
		{
			const cxxSurface *entity_ptr =
				&(entities.find(it->first)->second);
			this->add(P_INSTANCE_COMMA *entity_ptr, it->second);
		}
	}
}

cxxSurface::~cxxSurface()
{
}

bool
cxxSurface::get_related_phases()
{
	for (std::map < std::string, cxxSurfaceComp >::const_iterator it =
		 this->surfaceComps.begin(); it != this->surfaceComps.end(); ++it)
	{
		if ((*it).second.get_phase_name().size() == 0)
			continue;
		return (true);
	}
	return (false);
}

bool
cxxSurface::get_related_rate()
{
	for (std::map < std::string, cxxSurfaceComp >::const_iterator it =
		 this->surfaceComps.begin(); it != this->surfaceComps.end(); ++it)
	{
		if ((*it).second.get_rate_name().size() == 0)
			continue;
		return (true);
	}
	return (false);
}

struct surface *
cxxSurface::cxxSurface2surface(PHREEQC_PTR_ARG)
		//
		// Builds a surface structure from instance of cxxSurface 
		//
{
	struct surface *surface_ptr = P_INSTANCE_POINTER surface_alloc();

	surface_ptr->description = P_INSTANCE_POINTER string_duplicate (this->get_description().c_str());
	surface_ptr->n_user = this->n_user;
	surface_ptr->n_user_end = this->n_user_end;
	surface_ptr->new_def = FALSE;
	surface_ptr->type = this->type;
	surface_ptr->dl_type = this->dl_type;
	surface_ptr->sites_units = this->sites_units;
	//surface_ptr->diffuse_layer               = this->diffuse_layer;
	//surface_ptr->edl                         = this->edl;
	surface_ptr->only_counter_ions = this->only_counter_ions;
	//surface_ptr->donnan                      = this->donnan;  
	surface_ptr->thickness = this->thickness;
	surface_ptr->debye_lengths = 1.0;
	surface_ptr->solution_equilibria = FALSE;
	surface_ptr->n_solution = -2;
	surface_ptr->related_phases = (int) this->get_related_phases();
	surface_ptr->related_rate = (int) this->get_related_rate();
	surface_ptr->transport = this->transport;
	surface_ptr->debye_lengths = this->debye_lengths;
	surface_ptr->DDL_viscosity = this->DDL_viscosity;
	surface_ptr->DDL_limit = this->DDL_limit;

	// Surface comps
	surface_ptr->count_comps = (int) this->surfaceComps.size();
	surface_ptr->comps =
		(struct surface_comp *) P_INSTANCE_POINTER free_check_null(surface_ptr->comps);
	surface_ptr->comps =
		cxxSurfaceComp::cxxSurfaceComp2surface_comp(P_INSTANCE_COMMA this->surfaceComps);

	// Surface charge
	surface_ptr->charge =
		(struct surface_charge *) P_INSTANCE_POINTER free_check_null(surface_ptr->charge);
	//if (surface_ptr->edl == TRUE) {
	if (surface_ptr->type == DDL || surface_ptr->type == CD_MUSIC)
	{
		surface_ptr->count_charge = (int) this->surfaceCharges.size();
		surface_ptr->charge =
			cxxSurfaceCharge::cxxSurfaceCharge2surface_charge(P_INSTANCE_COMMA this->surfaceCharges);
	}
	else
	{
		surface_ptr->count_charge = 0;
	}
	// Need to fill in charge (number) in comps list
	if (surface_ptr->type != NO_EDL)
	{
		int i, j;
		for (i = 0; i < surface_ptr->count_comps; i++)
		{
			char *charge_name = P_INSTANCE_POINTER string_hsave(
				cxxSurfaceComp::get_charge_name(P_INSTANCE_COMMA surface_ptr->comps[i].formula).c_str());
			for (j = 0; j < surface_ptr->count_charge; j++)
			{
				if (charge_name == surface_ptr->charge[j].name)
				{
					surface_ptr->comps[i].charge = j;
					break;
				}
			}
			assert(j < surface_ptr->count_charge);
		}
	}

	return (surface_ptr);
}

void
cxxSurface::dump_xml(std::ostream & s_oss, unsigned int indent) const
{
	//const char    ERR_MESSAGE[] = "Packing surface message: %s, element not found\n";
	unsigned int i;
	s_oss.precision(DBL_DIG - 1);
	std::string indent0(""), indent1(""), indent2("");
	for (i = 0; i < indent; ++i)
		indent0.append(Utilities::INDENT);
	for (i = 0; i < indent + 1; ++i)
		indent1.append(Utilities::INDENT);
	for (i = 0; i < indent + 2; ++i)
		indent2.append(Utilities::INDENT);

	// Surface element and attributes
	s_oss << indent0;
	s_oss << "<surface " << std::endl;

	s_oss << indent1;
	//s_oss << "diffuse_layer=\"" << this->diffuse_layer << "\"" << std::endl;
	s_oss << "surface_type=\"" << this->type << "\"" << std::endl;

	s_oss << indent1;
	//s_oss << "edl=\"" << this->edl << "\"" << std::endl;
	s_oss << "dl_type=\"" << this->dl_type << "\"" << std::endl;

	s_oss << indent1;
	s_oss << "sites_units=\"" << this->sites_units << "\"" << std::endl;

	s_oss << indent1;
	s_oss << "only_counter_ions=\"" << this->
		only_counter_ions << "\"" << std::endl;

	//s_oss << indent1;
	//s_oss << "donnan=\"" << this->donnan << "\"" << std::endl;

	s_oss << indent1;
	s_oss << "thickness=\"" << this->thickness << "\"" << std::endl;

	s_oss << indent1;
	s_oss << "debye_lengths=\"" << this->debye_lengths << "\"" << std::endl;

	s_oss << indent1;
	s_oss << "DDL_viscosity=\"" << this->DDL_viscosity << "\"" << std::endl;

	s_oss << indent1;
	s_oss << "DDL_limit=\"" << this->DDL_limit << "\"" << std::endl;

	s_oss << indent1;
	s_oss << "transport=\"" << this->transport << "\"" << std::endl;

	// surface component structures
	s_oss << indent1;
	s_oss << "<component " << std::endl;
	{
		for (std::map < std::string, cxxSurfaceComp >::const_iterator it =
			 this->surfaceComps.begin(); it != this->surfaceComps.end(); ++it)
		{
			(*it).second.dump_xml(s_oss, indent + 2);
		}
	}
	// surface charge structures
	s_oss << indent1;
	s_oss << "<charge_component " << std::endl;
	for (std::map < std::string, cxxSurfaceCharge >::const_iterator it =
		 surfaceCharges.begin(); it != surfaceCharges.end(); ++it)
	{
		(*it).second.dump_xml(s_oss, indent + 2);
	}

	return;
}

void
cxxSurface::dump_raw(std::ostream & s_oss, unsigned int indent) const
{
	//const char    ERR_MESSAGE[] = "Packing surface message: %s, element not found\n";
	unsigned int i;
	s_oss.precision(DBL_DIG - 1);
	std::string indent0(""), indent1(""), indent2("");
	for (i = 0; i < indent; ++i)
		indent0.append(Utilities::INDENT);
	for (i = 0; i < indent + 1; ++i)
		indent1.append(Utilities::INDENT);
	for (i = 0; i < indent + 2; ++i)
		indent2.append(Utilities::INDENT);

	// Surface element and attributes
	s_oss << indent0;
	s_oss << "SURFACE_RAW       " << this->n_user << " " << this->
		description << std::endl;

	s_oss << indent1;
	//s_oss << "-diffuse_layer " << this->diffuse_layer << std::endl;
	s_oss << "-type " << this->type << std::endl;

	s_oss << indent1;
	//s_oss << "-edl " << this->edl << std::endl;
	s_oss << "-dl_type " << this->dl_type << std::endl;

	s_oss << indent1;
	s_oss << "-sites_units " << this->sites_units << std::endl;

	s_oss << indent1;
	s_oss << "-only_counter_ions " << this->only_counter_ions << std::endl;

	//s_oss << indent1;
	//s_oss << "-donnan " << this->donnan << std::endl;

	s_oss << indent1;
	s_oss << "-thickness " << this->thickness << std::endl;

	s_oss << indent1;
	s_oss << "-debye_lengths " << this->debye_lengths << std::endl;

	s_oss << indent1;
	s_oss << "-DDL_viscosity " << this->DDL_viscosity << std::endl;

	s_oss << indent1;
	s_oss << "-DDL_limit " << this->DDL_limit << std::endl;

	s_oss << indent1;
	s_oss << "-transport " << this->transport << std::endl;

	// surfaceComps structures
	for (std::map < std::string, cxxSurfaceComp >::const_iterator it =
		 surfaceComps.begin(); it != surfaceComps.end(); ++it)
	{
		s_oss << indent1;
		s_oss << "-component" << std::endl;
		(*it).second.dump_raw(s_oss, indent + 2);
	}
	// surface charge structures
	{
		for (std::map < std::string, cxxSurfaceCharge >::const_iterator it =
			 surfaceCharges.begin(); it != surfaceCharges.end(); ++it)
		{
			s_oss << indent1;
			s_oss << "-charge_component " << std::endl;
			(*it).second.dump_raw(s_oss, indent + 2);
		}
	}

	return;
}

void
cxxSurface::read_raw(PHREEQC_PTR_ARG_COMMA CParser & parser, bool check)
{
	static std::vector < std::string > vopts;
	int i = 0;
	if (vopts.empty())
	{
		vopts.reserve(15);
		vopts.push_back("diffuse_layer");	// 0 
		vopts.push_back("edl");	// 1 
		vopts.push_back("only_counter_ions");	// 2 
		vopts.push_back("donnan");	// 3 
		vopts.push_back("thickness");	// 4 
		vopts.push_back("component");	// 5
		vopts.push_back("charge_component");	// 6 
		vopts.push_back("type ");	// 7
		vopts.push_back("dl_type");	// 8 
		vopts.push_back("sites_units");	// 9 
		vopts.push_back("debye_lengths");	// 10
		vopts.push_back("ddl_viscosity");	// 11
		vopts.push_back("ddl_limit");	// 12
		vopts.push_back("transport");	// 13
	}

	std::istream::pos_type ptr;
	std::istream::pos_type next_char;
	std::string token;
	int opt_save;
	bool useLastLine(false);

	// Read surface number and description
	this->read_number_description(parser);

	opt_save = CParser::OPT_ERROR;
	//bool diffuse_layer_defined(false); 
	//bool edl_defined(false); 
	bool only_counter_ions_defined(false);
	//bool donnan_defined(false); 
	bool thickness_defined(false);
	bool type_defined(false);
	bool dl_type_defined(false);
	bool sites_units_defined(false);
	bool debye_lengths_defined(false);
	bool DDL_viscosity_defined(false);
	bool DDL_limit_defined(false);
	bool transport_defined(false);

	for (;;)
	{
		int opt;
		if (useLastLine == false)
		{
			opt = parser.get_option(vopts, next_char);
		}
		else
		{
			opt = parser.getOptionFromLastLine(vopts, next_char);
		}
		switch (opt)
		{
		case CParser::OPT_EOF:
			break;
		case CParser::OPT_KEYWORD:
			break;
		case CParser::OPT_DEFAULT:
		case CParser::OPT_ERROR:
			opt = CParser::OPT_EOF;
			parser.error_msg("Unknown input in SURFACE keyword.",
							 CParser::OT_CONTINUE);
			parser.error_msg(parser.line().c_str(), CParser::OT_CONTINUE);
			useLastLine = false;
			break;

		case 0:				// diffuse_layer
			parser.incr_input_error();
			parser.error_msg("Diffuse layer is obsolete, use -type.",
							 CParser::OT_CONTINUE);
			/*
			   if (!(parser.get_iss() >> this->diffuse_layer))
			   {
			   this->diffuse_layer = false;
			   parser.incr_input_error();
			   parser.error_msg("Expected boolean value for diffuse_layer.", CParser::OT_CONTINUE);
			   }
			   diffuse_layer_defined = true;
			 */
			useLastLine = false;
			break;

		case 1:				// edl
			parser.incr_input_error();
			parser.error_msg("-edl is obsolete, use -type.",
							 CParser::OT_CONTINUE);
			/*
			   if (!(parser.get_iss() >> this->edl))
			   {
			   this->edl = false;
			   parser.incr_input_error();
			   parser.error_msg("Expected boolean value for edl.", CParser::OT_CONTINUE);
			   }
			   edl_defined = true;
			 */
			useLastLine = false;
			break;

		case 2:				// only_counter_ions
			if (!(parser.get_iss() >> this->only_counter_ions))
			{
				this->only_counter_ions = false;
				parser.incr_input_error();
				parser.
					error_msg("Expected boolean value for only_counter_ions.",
							  CParser::OT_CONTINUE);
			}
			only_counter_ions_defined = true;
			useLastLine = false;
			break;

		case 3:				// donnan
			parser.incr_input_error();
			parser.error_msg("-Donnan is obsolete, use -dl_type.",
							 CParser::OT_CONTINUE);
			/*
			   if (!(parser.get_iss() >> this->donnan))
			   {
			   this->donnan = false;
			   parser.incr_input_error();
			   parser.error_msg("Expected boolean value for donnan.", CParser::OT_CONTINUE);
			   }
			   donnan_defined = true;
			 */
			useLastLine = false;
			break;

		case 4:				// thickness
			if (!(parser.get_iss() >> this->thickness))
			{
				this->thickness = 0.0;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for thickness.",
								 CParser::OT_CONTINUE);
			}
			thickness_defined = true;
			useLastLine = false;
			break;

		case 5:				// component
			{
				cxxSurfaceComp ec;

				// preliminary read
#ifdef SKIP
				std::istream::pos_type pos = parser.tellg();
				CParser::ECHO_OPTION eo = parser.get_echo_file();
				parser.set_echo_file(CParser::EO_NONE);
				CParser::ECHO_OPTION eo_s = parser.get_echo_stream();
				parser.set_echo_stream(CParser::EO_NONE);
				ec.read_raw(parser, false);
				parser.set_echo_file(eo);
				parser.set_echo_stream(eo_s);
				parser.seekg(pos).clear();
				parser.seekg(pos);

				if (this->surfaceComps.find(ec.get_formula()) != this->surfaceComps.end())
				{
					cxxSurfaceComp & comp = this->surfaceComps.find(ec.get_formula())->second;
					comp.read_raw(parser, false);
				}
				else
				{
					cxxSurfaceComp ec1;
					ec1.read_raw(parser, false);
					std::string str(ec1.get_formula());
					this->surfaceComps[str] = ec1;
				}
#endif
				parser.set_accumulate(true);
				ec.read_raw(P_INSTANCE_COMMA parser, false);
				parser.set_accumulate(false);
				std::istringstream is(parser.get_accumulated());
				CParser reread(P_INSTANCE_COMMA is);
				reread.set_echo_file(CParser::EO_NONE);
				reread.set_echo_stream(CParser::EO_NONE);
				if (this->surfaceComps.find(ec.get_formula()) != this->surfaceComps.end())
				{
					cxxSurfaceComp & comp = this->surfaceComps.find(ec.get_formula())->second;
					comp.read_raw(P_INSTANCE_COMMA reread, false);
				}
				else
				{
					cxxSurfaceComp ec1;
					ec1.read_raw(P_INSTANCE_COMMA reread, false);
					std::string str(ec1.get_formula());
					this->surfaceComps[str] = ec1;
				}
			}
			useLastLine = true;
			break;

		case 6:				// charge_component
			{
				cxxSurfaceCharge ec;

				// preliminary read
#ifdef SKIP
				std::istream::pos_type pos = parser.tellg();
				CParser::ECHO_OPTION eo = parser.get_echo_file();
				parser.set_echo_file(CParser::EO_NONE);
				CParser::ECHO_OPTION eo_s = parser.get_echo_stream();
				parser.set_echo_stream(CParser::EO_NONE);
				ec.read_raw(parser, false);
				parser.set_echo_file(eo);
				parser.set_echo_stream(eo_s);
				parser.seekg(pos).clear();
				parser.seekg(pos);

				if (this->surfaceCharges.find(ec.get_name()) != this->surfaceCharges.end())
				{
					cxxSurfaceCharge & comp = this->surfaceCharges.find(ec.get_name())->second;
					comp.read_raw(parser, false);
				}
				else
				{
					cxxSurfaceCharge ec1;
					ec1.read_raw(parser, false);
					std::string str(ec1.get_name());
					this->surfaceCharges[str] = ec1;
				}
#endif
				parser.set_accumulate(true);
				ec.read_raw(P_INSTANCE_COMMA parser, false);
				parser.set_accumulate(false);
				std::istringstream is(parser.get_accumulated());
				CParser reread(P_INSTANCE_COMMA is);
				reread.set_echo_file(CParser::EO_NONE);
				reread.set_echo_stream(CParser::EO_NONE);
				if (this->surfaceCharges.find(ec.get_name()) != this->surfaceCharges.end())
				{
					cxxSurfaceCharge & comp = this->surfaceCharges.find(ec.get_name())->second;
					comp.read_raw(P_INSTANCE_COMMA reread, false);
				}
				else
				{
					cxxSurfaceCharge ec1;
					ec1.read_raw(P_INSTANCE_COMMA reread, false);
					std::string str(ec1.get_name());
					this->surfaceCharges[str] = ec1;
				}
			}
			useLastLine = true;
			break;
		case 7:				// type
			i = 0;
			if (!(parser.get_iss() >> i))
			{
				this->type = NO_EDL;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for type.",
								 CParser::OT_CONTINUE);
			}
			this->type = (SURFACE_TYPE) i;
			type_defined = true;
			useLastLine = false;
			break;
		case 8:				// dl_type
			i = 0;
			if (!(parser.get_iss() >> i))
			{
				this->dl_type = NO_DL;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for dl_type.",
								 CParser::OT_CONTINUE);
			}
			this->dl_type = (DIFFUSE_LAYER_TYPE) i;
			dl_type_defined = true;
			useLastLine = false;
			break;
		case 9:				// sites_units
			i = 0;
			if (!(parser.get_iss() >> i))
			{
				this->sites_units = SITES_ABSOLUTE;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for sites_units.",
								 CParser::OT_CONTINUE);
			}
			this->sites_units = (SITES_UNITS) i;
			sites_units_defined = true;
			useLastLine = false;
			break;

		case 10:				// debye_lengths
			if (!(parser.get_iss() >> this->debye_lengths))
			{
				this->debye_lengths = 0.0;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for debye_lengths.",
								 CParser::OT_CONTINUE);
			}
			debye_lengths_defined = true;
			useLastLine = false;
			break;

		case 11:				// DDL_viscosity
			if (!(parser.get_iss() >> this->DDL_viscosity))
			{
				this->DDL_viscosity = 0.0;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for DDL_viscosity.",
								 CParser::OT_CONTINUE);
			}
			DDL_viscosity_defined = true;
			useLastLine = false;
			break;

		case 12:				// DDL_limit
			if (!(parser.get_iss() >> this->DDL_limit))
			{
				this->DDL_limit = 0.0;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for DDL_limit.",
								 CParser::OT_CONTINUE);
			}
			DDL_limit_defined = true;
			useLastLine = false;
			break;

		case 13:				// transport
			if (!(parser.get_iss() >> this->transport))
			{
				this->transport = false;
				parser.incr_input_error();
				parser.error_msg("Expected boolean value for transport.",
								 CParser::OT_CONTINUE);
			}
			transport_defined = true;
			useLastLine = false;
			break;
		}

		if (opt == CParser::OPT_EOF || opt == CParser::OPT_KEYWORD)
			break;
	}
	if (check)
	{
		// members that must be defined
		/*
		if (diffuse_layer_defined == false) {
		parser.incr_input_error();
		parser.error_msg("Diffuse_layer not defined for SURFACE_RAW input.", CParser::OT_CONTINUE);
		}
		if (edl_defined == false) {
		parser.incr_input_error();
		parser.error_msg("Edl not defined for SURFACE_RAW input.", CParser::OT_CONTINUE);
		}
		*/
		if (only_counter_ions_defined == false)
		{
			parser.incr_input_error();
			parser.
				error_msg("Only_counter_ions not defined for SURFACE_RAW input.",
				CParser::OT_CONTINUE);
		}
		/*
		if (donnan_defined == false) {
		parser.incr_input_error();
		parser.error_msg("Donnan not defined for SURFACE_RAW input.", CParser::OT_CONTINUE);
		}
		*/
		if (thickness_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("Thickness not defined for SURFACE_RAW input.",
				CParser::OT_CONTINUE);
		}
		if (type_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("Surface type not defined for SURFACE_RAW input.",
				CParser::OT_CONTINUE);
		}
		if (dl_type_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("Dl_type not defined for SURFACE_RAW input.",
				CParser::OT_CONTINUE);
		}
		if (sites_units_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("Sites_units not defined for SURFACE_RAW input.",
				CParser::OT_CONTINUE);
		}
		if (debye_lengths_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("Debye_lengths not defined for SURFACE_RAW input.",
				CParser::OT_CONTINUE);
		}
		if (DDL_viscosity_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("DDL_viscosity not defined for SURFACE_RAW input.",
				CParser::OT_CONTINUE);
		}
		if (DDL_limit_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("DDL_limit not defined for SURFACE_RAW input.",
				CParser::OT_CONTINUE);
		}
		if (transport_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("Transport not defined for SURFACE_RAW input.",
				CParser::OT_CONTINUE);
		}
	}
}

#ifdef USE_MPI
/* ---------------------------------------------------------------------- */
void
cxxSurface::mpi_pack(std::vector < int >&ints, std::vector < double >&doubles)
/* ---------------------------------------------------------------------- */
{
	/* int n_user; */
	ints.push_back(this->n_user);
	ints.push_back((int) this->surfaceComps.size());
	for (std::map < std::string, cxxSurfaceComp >::iterator it =
		 this->surfaceComps.begin(); it != this->surfaceComps.end(); it++)
	{
		(*it).second.mpi_pack(ints, doubles);
	}
	ints.push_back((int) this->surfaceCharges.size());
	for (std::map < std::string, cxxSurfaceCharge >::iterator it =
		 this->surfaceCharges.begin(); it != this->surfaceCharges.end(); it++)
	{
		(*it).second.mpi_pack(ints, doubles);
	}
	//ints.push_back((int) this->diffuse_layer);
	//ints.push_back((int) this->edl);
	ints.push_back((int) this->type);
	ints.push_back((int) this->dl_type);
	ints.push_back((int) this->sites_units);

	ints.push_back((int) this->only_counter_ions);
	//ints.push_back((int) this->donnan);
	doubles.push_back(this->thickness);
	doubles.push_back(this->debye_lengths);
	doubles.push_back(this->DDL_viscosity);
	doubles.push_back(this->DDL_limit);
	ints.push_back((int) this->transport);
}

/* ---------------------------------------------------------------------- */
void
cxxSurface::mpi_unpack(int *ints, int *ii, double *doubles, int *dd)
/* ---------------------------------------------------------------------- */
{
	int i = *ii;
	int d = *dd;
	/* int n_user; */
	this->n_user = ints[i++];
	this->n_user_end = this->n_user;
	this->description = " ";

	int count = ints[i++];
	this->surfaceComps.clear();
	for (int n = 0; n < count; n++)
	{
		cxxSurfaceComp sc;
		sc.mpi_unpack(ints, &i, doubles, &d);
		std::string str(sc.get_formula());
		this->surfaceComps[str] = sc;
	}
	count = ints[i++];
	this->surfaceCharges.clear();
	for (int n = 0; n < count; n++)
	{
		cxxSurfaceCharge sc;
		sc.mpi_unpack(ints, &i, doubles, &d);
		std::string str(sc.get_name());
		this->surfaceCharges[str] = sc;
	}
	//this->diffuse_layer = (bool) ints[i++];
	//this->edl = (bool) ints[i++];
	this->type = (SURFACE_TYPE) ints[i++];
	this->dl_type = (DIFFUSE_LAYER_TYPE) ints[i++];
	this->sites_units = (SITES_UNITS) ints[i++];
	this->only_counter_ions = (ints[i++] == TRUE);
	//this->donnan = (bool) ints[i++];
	this->thickness = doubles[d++];
	this->debye_lengths = doubles[d++];
	this->DDL_viscosity = doubles[d++];
	this->DDL_limit = doubles[d++];
	this->transport = (ints[i++] == TRUE);
	*ii = i;
	*dd = d;
}
#endif

void
cxxSurface::totalize()
{
	this->totals.clear();
	// component structures
	for (std::map < std::string, cxxSurfaceComp >::const_iterator it =
		 surfaceComps.begin(); it != surfaceComps.end(); ++it)
	{
		this->totals.add_extensive((*it).second.get_totals(), 1.0);
		this->totals.add("Charge", (*it).second.get_charge_balance());
	}
	return;
}
#ifdef SKIP
void
cxxSurface::add(const cxxSurface & addee, double extensive)
		//
		// Add surface to "this" exchange
		//
{
	//std::list<cxxSurfaceComp> surfaceComps;
	//std::list<cxxSurfaceCharge> surfaceCharges;
	//enum SURFACE_TYPE type;
	//enum DIFFUSE_LAYER_TYPE dl_type;
	//enum SITES_UNITS sites_units;
	//bool only_counter_ions;
	//double thickness;
	//double debye_lengths;
	//double DDL_viscosity;
	//double DDL_limit;
	//bool transport;
	if (extensive == 0.0)
		return;
	if (this->surfaceComps.size() == 0)
	{
		//enum SURFACE_TYPE type;
		this->type = addee.type;
		//enum DIFFUSE_LAYER_TYPE dl_type;
		this->dl_type = addee.dl_type;
		//enum SITES_UNITS sites_units;
		this->sites_units = addee.sites_units;
		//bool only_counter_ions;
		this->only_counter_ions = addee.only_counter_ions;
		//double thickness;
		this->thickness = addee.thickness;
		//double debye_lengths;
		this->debye_lengths = addee.debye_lengths;
		//double DDL_viscosity;
		this->DDL_viscosity = addee.DDL_viscosity;
		//double DDL_limit;
		this->DDL_limit = addee.DDL_limit;
		//bool transport;
		this->transport = addee.transport;
	}
	//std::list<cxxSurfaceComp> surfaceComps;

	for (std::list < cxxSurfaceComp >::const_iterator itadd =
		 addee.surfaceComps.begin(); itadd != addee.surfaceComps.end();
		 ++itadd)
	{
		bool found = false;
		for (std::list < cxxSurfaceComp >::iterator it =
			 this->surfaceComps.begin(); it != this->surfaceComps.end(); ++it)
		{
			if (it->get_formula() == itadd->get_formula())
			{
				it->add((*itadd), extensive);
				found = true;
				break;
			}
		}
		if (!found)
		{
			cxxSurfaceComp entity = *itadd;
			entity.multiply(extensive);
			//exc.add(*itadd, extensive);
			this->surfaceComps.push_back(entity);
		}
	}
	//std::list<cxxSurfaceCharge> surfaceCharges;
	for (std::list < cxxSurfaceCharge >::const_iterator itadd =
		 addee.surfaceCharges.begin(); itadd != addee.surfaceCharges.end();
		 ++itadd)
	{
		bool found = false;
		for (std::list < cxxSurfaceCharge >::iterator it =
			 this->surfaceCharges.begin(); it != this->surfaceCharges.end();
			 ++it)
		{
			if (it->get_name() == itadd->get_name())
			{
				it->add((*itadd), extensive);
				found = true;
				break;
			}
		}
		if (!found)
		{
			cxxSurfaceCharge entity = *itadd;
			entity.multiply(extensive);
			//exc.add(*itadd, extensive);
			this->surfaceCharges.push_back(entity);
		}
	}

}
#endif
void
cxxSurface::add(PHREEQC_PTR_ARG_COMMA const cxxSurface & addee, double extensive)
		//
		// Add surface to "this" exchange
		//
{
	if (extensive == 0.0)
		return;
	if (this->surfaceComps.size() == 0)
	{
		//enum SURFACE_TYPE type;
		this->type = addee.type;
		//enum DIFFUSE_LAYER_TYPE dl_type;
		this->dl_type = addee.dl_type;
		//enum SITES_UNITS sites_units;
		this->sites_units = addee.sites_units;
		//bool only_counter_ions;
		this->only_counter_ions = addee.only_counter_ions;
		//double thickness;
		this->thickness = addee.thickness;
		//double debye_lengths;
		this->debye_lengths = addee.debye_lengths;
		//double DDL_viscosity;
		this->DDL_viscosity = addee.DDL_viscosity;
		//double DDL_limit;
		this->DDL_limit = addee.DDL_limit;
		//bool transport;
		this->transport = addee.transport;
	}

	//std::map <std::string, cxxSurfaceComp> surfaceComps;
	for (std::map < std::string, cxxSurfaceComp >::const_iterator itadd =
		 addee.surfaceComps.begin(); itadd != addee.surfaceComps.end();
		 ++itadd)
	{
		std::map < std::string, cxxSurfaceComp >::iterator it = this->surfaceComps.find((*itadd).first);
		if (it != this->surfaceComps.end())
		{
			(*it).second.add(P_INSTANCE_COMMA (*itadd).second, extensive);
		}
		else
		{
			cxxSurfaceComp entity = (*itadd).second;
			entity.multiply(extensive);
			std::string str(entity.get_formula());
			this->surfaceComps[str] = entity;
		}
	}

	//std::map < std::string, cxxSurfaceCharge > surfaceCharges;
	for (std::map < std::string, cxxSurfaceCharge >::const_iterator itadd =
		 addee.surfaceCharges.begin(); itadd != addee.surfaceCharges.end();
		 ++itadd)
	{
		std::map < std::string, cxxSurfaceCharge >::iterator it = this->surfaceCharges.find((*itadd).first);
		if (it != this->surfaceCharges.end())
		{
			(*it).second.add( (*itadd).second, extensive);
		}
		else
		{
			cxxSurfaceCharge entity = (*itadd).second;
			entity.multiply(extensive);
			std::string str(entity.get_name());
			this->surfaceCharges[str] = entity;
		}
	}
}
