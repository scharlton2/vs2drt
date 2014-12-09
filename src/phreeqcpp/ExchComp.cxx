// ExchComp.cxx: implementation of the cxxExchComp class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#pragma warning(disable : 4786)	// disable truncation warning (Only used by debugger)
#endif
#include <iostream>				// std::cout std::cerr
#include <cassert>				// assert
#include <algorithm>			// std::sort
#include <float.h>

#include "Utils.h"				// define first
#if !defined(PHREEQC_CLASS)
#define EXTERNAL extern
#include "global.h"
#else
#include "Phreeqc.h"
#endif
#include "ExchComp.h"
#include "Dictionary.h"
#include "phqalloc.h"
#include "phrqproto.h"
#include "output.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

cxxExchComp::cxxExchComp()
	//
	// default constructor for cxxExchComp 
	//
{
	moles = 0.0;
	formula_totals.type = cxxNameDouble::ND_ELT_MOLES;
	totals.type = cxxNameDouble::ND_ELT_MOLES;
	la = 0.0;
	charge_balance = 0.0;
	//phase_name = NULL;
	phase_proportion = 0.0;
	//rate_name = NULL;
	formula_z = 0.0;
}

cxxExchComp::cxxExchComp(struct exch_comp * exch_comp_ptr)
	//
	// constructor for cxxExchComp from struct exch_comp
	//
:
formula_totals(exch_comp_ptr->formula_totals), totals(exch_comp_ptr->totals)
{
	this->set_formula(exch_comp_ptr->formula);
	moles = exch_comp_ptr->moles;
	// totals in constructor
	//formula_totals in constructor
	la = exch_comp_ptr->la;
	charge_balance = exch_comp_ptr->charge_balance;
	this->set_phase_name(exch_comp_ptr->phase_name);
	phase_proportion = exch_comp_ptr->phase_proportion;
	this->set_rate_name(exch_comp_ptr->rate_name);
	formula_z = exch_comp_ptr->formula_z;
}

#ifdef SKIP
cxxExchComp::cxxExchComp(std::vector < cxxExchComp > &ec_vector,
						 std::vector < double >&f_vector)
		//
		// constructor for cxxExchComp from mixing 
		//
{
	if (ec_vector.size() <= 0)
		return;
	//
	//  check consistency
	//
	std::vector < double >::iterator it_f;
	std::vector < cxxExchComp >::iterator it_ec;
	// set fixed variables
	it_ec = ec_vector.begin();
	this->formula = it_ec->formula;
	this->formula_totals = it_ec->formula_totals;
	this->formula_z = it_ec->formula_z;
	this->phase_name = it_ec->phase_name;
	this->rate_name = it_ec->rate_name;
	it_ec++;
	for (; it_ec != ec_vector.end(); it_ec++)
	{
		if (it_ec->formula != this->formula ||
			it_ec->formula_z != this->formula_z ||
			it_ec->phase_name != this->phase_name ||
			this->rate_name != this->rate_name)
		{
			error_msg
				("Mixing exchange components. Formula, z, phase_name, or rate_name did not match",
				 STOP);
		}
	}
	// calculate sum of extensive factors
	double sum_extensive = 0;
	for (it_f = f_vector.begin(); it_f != f_vector.end(); it_f++)
	{
		sum_extensive += *it_f;
	}
	this->moles = 0;
	this->la = 0;
	this->charge_balance = 0;
	this->phase_proportion = 0;
	this->totals.clear();
	this->totals.type = cxxNameDouble::ND_ELT_MOLES;
	it_ec = ec_vector.begin();
	it_f = f_vector.begin();
	for (; it_ec != ec_vector.end();)
	{
		double extensive = *it_f;
		double intensive = extensive / sum_extensive;
		this->moles += it_ec->moles * extensive;
		this->la += it_ec->la * intensive;
		this->charge_balance += it_ec->charge_balance * extensive;
		this->phase_proportion += it_ec->phase_proportion * intensive;
		this->totals.add_extensive(it_ec->totals, extensive);
		it_ec++;
		it_f++;
	}
}
#endif
cxxExchComp::~cxxExchComp()
{
}

struct master *
cxxExchComp::get_master(PHREEQC_PTR_ARG)
{
	struct master *master_ptr = NULL;
	for (std::map < std::string, double >::iterator it =
		 totals.begin(); it != totals.end(); it++)
	{

		/* Find master species */
		char *eltName = P_INSTANCE_POINTER string_hsave(it->first.c_str());
		assert(it->first.size() != 0);
		struct element *elt_ptr = P_INSTANCE_POINTER element_store(eltName);
		if (elt_ptr->master == NULL)
		{
			std::ostringstream error_oss;
			error_oss << "Master species not in data base for " << elt_ptr->
				name << std::endl;
			//Utilities::error_msg(error_oss.str(), STOP);
			P_INSTANCE_POINTER error_msg(error_oss.str().c_str(), CONTINUE);
			return (NULL);
		}
		if (elt_ptr->master->type != EX)
			continue;
		master_ptr = elt_ptr->master;
		break;
	}
	if (master_ptr == NULL)
	{
		for (std::map < std::string, double >::iterator it =
			 this->formula_totals.begin(); it != formula_totals.end(); it++)
		{

			/* Find master species */
			char *eltName = P_INSTANCE_POINTER string_hsave(it->first.c_str());
			assert(it->first.size() != 0);
			struct element *elt_ptr = P_INSTANCE_POINTER element_store(eltName);
			if (elt_ptr->master == NULL)
			{
				std::ostringstream error_oss;
				error_oss << "Master species not in data base for " <<
					elt_ptr->name << std::endl;
				//Utilities::error_msg(error_oss.str(), STOP);
				P_INSTANCE_POINTER error_msg(error_oss.str().c_str(), CONTINUE);
				return (NULL);
			}
			if (elt_ptr->master->type != EX)
				continue;
			master_ptr = elt_ptr->master;
			break;
		}
	}
	if (master_ptr == NULL)
	{
		std::ostringstream error_oss;
		error_oss <<
			"Exchange formula does not contain an exchange master species, "
			<< this->formula << std::endl;
		//Utilities::error_msg(error_oss.str(), CONTINUE);
		P_INSTANCE_POINTER error_msg(error_oss.str().c_str(), CONTINUE);

		std::ostringstream oss;
		this->dump_raw(oss, 0);
		std::cerr << oss.str();

	}
	return (master_ptr);
}

struct exch_comp *
	cxxExchComp::cxxExchComp2exch_comp(PHREEQC_PTR_ARG_COMMA std::map < std::string, cxxExchComp > &el)
		//
		// Builds exch_comp structure from of cxxExchComp 
		//
{
	struct exch_comp *exch_comp_ptr =
		(struct exch_comp *)
		P_INSTANCE_POINTER PHRQ_malloc((size_t) (el.size() * sizeof(struct exch_comp)));
	if (exch_comp_ptr == NULL)
		P_INSTANCE_POINTER malloc_error();

	int i = 0;
	for (std::map < std::string, cxxExchComp >::iterator it = el.begin(); it != el.end();
		 ++it)
	{
		if ((*it).second.formula.size() == 0)
			exch_comp_ptr[i].formula = NULL;
		else
			exch_comp_ptr[i].formula = P_INSTANCE_POINTER string_hsave((*it).second.formula.c_str());
		exch_comp_ptr[i].formula_z = (*it).second.formula_z;
		exch_comp_ptr[i].totals = (*it).second.totals.elt_list(P_INSTANCE);
		exch_comp_ptr[i].moles = (*it).second.moles;
		exch_comp_ptr[i].formula_totals = (*it).second.formula_totals.elt_list(P_INSTANCE);
		exch_comp_ptr[i].la = (*it).second.la;
		exch_comp_ptr[i].charge_balance = (*it).second.charge_balance;
		if ((*it).second.phase_name.size() == 0)
			exch_comp_ptr[i].phase_name = NULL;
		else
			exch_comp_ptr[i].phase_name = P_INSTANCE_POINTER string_hsave((*it).second.phase_name.c_str());
		exch_comp_ptr[i].phase_proportion = (*it).second.phase_proportion;
		if ((*it).second.rate_name.size() == 0)
			exch_comp_ptr[i].rate_name = NULL;
		else
			exch_comp_ptr[i].rate_name = P_INSTANCE_POINTER string_hsave((*it).second.rate_name.c_str());
		exch_comp_ptr[i].master = (*it).second.get_master(P_INSTANCE);
		i++;
	}
	return (exch_comp_ptr);
}

void
cxxExchComp::dump_xml(std::ostream & s_oss, unsigned int indent) const
{
	//const char    ERR_MESSAGE[] = "Packing exch_comp message: %s, element not found\n";
	unsigned int i;
	s_oss.precision(DBL_DIG - 1);
	std::string indent0(""), indent1(""), indent2("");
	for (i = 0; i < indent; ++i)
		indent0.append(Utilities::INDENT);
	for (i = 0; i < indent + 1; ++i)
		indent1.append(Utilities::INDENT);
	for (i = 0; i < indent + 2; ++i)
		indent2.append(Utilities::INDENT);

	// Exch_Comp element and attributes

	s_oss << indent0 << "formula=\"" << this->formula << "\"" << std::endl;
	s_oss << indent0 << "formula_z=\"" << this->
		formula_z << "\"" << std::endl;
	s_oss << indent0 << "moles=\"" << this->moles << "\"" << std::endl;
	s_oss << indent0 << "la=\"" << this->la << "\"" << std::endl;
	s_oss << indent0 << "charge_balance=\"" << this->
		charge_balance << "\"" << std::endl;
	if (this->phase_name.size() != 0)
	{
		s_oss << indent0 << "phase_name=\"" << this->phase_name << "\"" << std::endl;
	}
	if (this->rate_name.size() != 0)
	{
		s_oss << indent0 << "rate_name=\"" << this->
			rate_name << "\"" << std::endl;
	}
	s_oss << indent0 << "phase_proportion=\"" << this->
		phase_proportion << "\"" << std::endl;

	// totals
	s_oss << indent0;
	s_oss << "<totals " << std::endl;
	this->totals.dump_xml(s_oss, indent + 1);

	// formula_totals
	s_oss << indent0;
	s_oss << "<formula_totals " << std::endl;
	this->formula_totals.dump_xml(s_oss, indent + 1);
}

void
cxxExchComp::dump_raw(std::ostream & s_oss, unsigned int indent) const
{
	//const char    ERR_MESSAGE[] = "Packing exch_comp message: %s, element not found\n";
	unsigned int i;
	s_oss.precision(DBL_DIG - 1);
	std::string indent0(""), indent1(""), indent2("");
	for (i = 0; i < indent; ++i)
		indent0.append(Utilities::INDENT);
	for (i = 0; i < indent + 1; ++i)
		indent1.append(Utilities::INDENT);
	for (i = 0; i < indent + 2; ++i)
		indent2.append(Utilities::INDENT);

	// Exch_Comp element and attributes

	s_oss << indent0 << "-formula               " << this->formula << std::endl;

	//s_oss << indent1 << "# critical values" << std::endl;

	// totals
	s_oss << indent1;
	s_oss << "-totals" << std::endl;
	this->totals.dump_raw(s_oss, indent + 2);

	s_oss << indent1 << "-charge_balance        " << this->charge_balance << std::endl;

	//s_oss << indent1 << "# Noncritical values" << std::endl;
	s_oss << indent1 << "-moles                 " << this->moles << std::endl;
	s_oss << indent1 << "-la                    " << this->la << std::endl;

	if (this->phase_name.size() != 0)
	{
		s_oss << indent1 << "-phase_name            " << this->phase_name << std::endl;
	}
	if (this->rate_name.size() != 0)
	{
		s_oss << indent1 << "-rate_name             " << this->rate_name << std::endl;
	}
	s_oss << indent1 << "-phase_proportion      " << this->phase_proportion << std::endl;
	s_oss << indent1 << "-formula_z             " << this->formula_z << std::endl;

	// formula_totals
	s_oss << indent1;
	s_oss << "-formula_totals" << std::endl;
	this->formula_totals.dump_raw(s_oss, indent + 2);

}

void
cxxExchComp::read_raw(PHREEQC_PTR_ARG_COMMA CParser & parser, bool check)
{
	std::string str;

	static std::vector < std::string > vopts;
	if (vopts.empty())
	{
		vopts.reserve(10);
		vopts.push_back("formula");	// 0 
		vopts.push_back("moles");	// 1
		vopts.push_back("la");	// 2 
		vopts.push_back("charge_balance");	// 3 
		vopts.push_back("phase_name");	// 4 
		vopts.push_back("rate_name");	// 5 
		vopts.push_back("formula_z");	// 6
		vopts.push_back("phase_proportion");	// 7 
		vopts.push_back("totals");	// 8
		vopts.push_back("formula_totals");	// 9
	}

	std::istream::pos_type ptr;
	std::istream::pos_type next_char;
	std::string token;
	int opt_save;

	opt_save = CParser::OPT_ERROR;
	bool formula_defined(false);
	bool moles_defined(false);
	bool la_defined(false);
	bool charge_balance_defined(false);
	bool formula_z_defined(false);

	for (;;)
	{
		int opt = parser.get_option(vopts, next_char);
		if (opt == CParser::OPT_DEFAULT)
		{
			opt = opt_save;
		}

		switch (opt)
		{
		case CParser::OPT_EOF:
			break;
		case CParser::OPT_KEYWORD:
			break;
		case CParser::OPT_DEFAULT:
		case CParser::OPT_ERROR:
			opt = CParser::OPT_KEYWORD;
			// Allow return to Exchange for more processing
			//parser.error_msg("Unknown input in EXCH_COMP read.", CParser::OT_CONTINUE);
			//parser.error_msg(parser.line().c_str(), CParser::OT_CONTINUE);
			break;

		case 0:				// formula
			if (!(parser.get_iss() >> str))
			{
				this->formula.clear();
				parser.incr_input_error();
				parser.error_msg("Expected string value for formula.",
								 CParser::OT_CONTINUE);
			}
			else
			{
				this->formula = str;
			}
			formula_defined = true;
			break;

		case 1:				// moles
			if (!(parser.get_iss() >> this->moles))
			{
				this->moles = 0;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for moles.",
								 CParser::OT_CONTINUE);
			}
			moles_defined = true;
			break;

		case 2:				// la
			if (!(parser.get_iss() >> this->la))
			{
				this->la = 0;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for la.",
								 CParser::OT_CONTINUE);
			}
			la_defined = true;
			break;

		case 3:				// charge_balance
			if (!(parser.get_iss() >> this->charge_balance))
			{
				this->charge_balance = 0;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for charge_balance.",
								 CParser::OT_CONTINUE);
			}
			charge_balance_defined = true;
			break;

		case 4:				// phase_name
			if (!(parser.get_iss() >> str))
			{
				this->phase_name.clear();
				parser.incr_input_error();
				parser.error_msg("Expected string value for phase_name.",
								 CParser::OT_CONTINUE);
			}
			else
			{
				this->phase_name = str;
			}
			break;

		case 5:				// rate_name
			if (!(parser.get_iss() >> str))
			{
				this->rate_name.clear();
				parser.incr_input_error();
				parser.error_msg("Expected string value for rate_name.",
								 CParser::OT_CONTINUE);
			}
			else
			{
				this->rate_name = str;
			}
			break;

		case 6:				// formula_z
			if (!(parser.get_iss() >> this->formula_z))
			{
				this->formula_z = 0;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for formula_z.",
								 CParser::OT_CONTINUE);
			}
			formula_z_defined = true;
			break;

		case 7:				// phase_proportion
			if (!(parser.get_iss() >> this->phase_proportion))
			{
				this->phase_proportion = 0;
				parser.incr_input_error();
				parser.
					error_msg("Expected numeric value for phase_proportion.",
							  CParser::OT_CONTINUE);
			}
			break;

		case 8:				// totals
			if (this->totals.read_raw(P_INSTANCE_COMMA parser, next_char) !=
				CParser::PARSER_OK)
			{
				parser.incr_input_error();
				parser.
					error_msg
					("Expected element name and molality for ExchComp totals.",
					 CParser::OT_CONTINUE);
			}
			opt_save = 8;
			break;

		case 9:				// formula_totals
			if (this->formula_totals.read_raw(P_INSTANCE_COMMA parser, next_char) !=
				CParser::PARSER_OK)
			{
				parser.incr_input_error();
				parser.
					error_msg
					("Expected element name and molality for ExchComp formula totals.",
					 CParser::OT_CONTINUE);
			}
			opt_save = 9;
			break;
		}
		if (opt == CParser::OPT_EOF || opt == CParser::OPT_KEYWORD)
			break;
	}
	if (check)
	{
		// members that must be defined
		if (formula_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("Formula not defined for ExchComp input.",
				CParser::OT_CONTINUE);
		}
		if (moles_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("Moles not defined for ExchComp input.",
				CParser::OT_CONTINUE);
		}
		if (la_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("La not defined for ExchComp input.",
				CParser::OT_CONTINUE);
		}
		if (charge_balance_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("Charge_balance not defined for ExchComp input.",
				CParser::OT_CONTINUE);
		}
		if (formula_z_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("Formula_z not defined for ExchComp input.",
				CParser::OT_CONTINUE);
		}
	}
}
void
cxxExchComp::add(PHREEQC_PTR_ARG_COMMA const cxxExchComp & addee, double extensive)
{
	double ext1, ext2, f1, f2;
	if (extensive == 0.0)
		return;
	if (addee.formula.size() == 0)
		return;
	// this and addee must have same formula
	// otherwise generate a new exchcomp with multiply

	ext1 = this->moles;
	ext2 = addee.moles * extensive;
	if (ext1 + ext2 != 0)
	{
		f1 = ext1 / (ext1 + ext2);
		f2 = ext2 / (ext1 + ext2);
	}
	else
	{
		f1 = 0.5;
		f2 = 0.5;
	}

	//char * formula;
	//cxxNameDouble formula_totals;
	if (this->formula.size() == 0 && addee.formula.size() == 0)
	{
		return;
	}
	assert(this->formula == addee.formula);
	assert(this->formula_z == addee.formula_z);
	if (this->formula.size() == 0 && addee.formula.size() != 0)
	{
		this->formula = addee.formula;
		this->formula_totals = addee.formula_totals;
	}
	//double moles;
	this->moles += addee.moles * extensive;
	//cxxNameDouble totals; 
	this->totals.add_extensive(addee.totals, extensive);
	//double la;
	this->la = f1 * this->la + f2 * addee.la;
	//double charge_balance;
	this->charge_balance += addee.charge_balance * extensive;
	//char   *phase_name;

	if (this->phase_name != addee.phase_name)
	{
		std::ostringstream oss;
		oss <<
			"Can not mix two exchange components with same formula and different related phases, "
			<< this->formula;
		P_INSTANCE_POINTER error_msg(oss.str().c_str(), CONTINUE);
		P_INSTANCE_POINTER input_error++;
		return;
	}
	else if (this->phase_name.size() != 0)
	{
		this->phase_proportion =
			this->phase_proportion * f1 + addee.phase_proportion * f2;
	}
	//char   *rate_name;
	if (this->rate_name != addee.rate_name)
	{
		std::ostringstream oss;
		oss <<
			"Can not mix two exchange components with same formula and different related kinetics, "
			<< this->formula;
		P_INSTANCE_POINTER error_msg(oss.str().c_str(), CONTINUE);
		P_INSTANCE_POINTER input_error++;
		return;
	}
	else if (this->rate_name.size() != 0)
	{
		//double phase_proportion;
		this->phase_proportion =
			this->phase_proportion * f1 + addee.phase_proportion * f2;
	}
	if ((this->rate_name.size() != 0 && addee.phase_name.size() != 0) ||
		(this->phase_name.size() != 0 && addee.rate_name.size() != 0))
	{
		std::ostringstream oss;
		oss <<
			"Can not mix exchange components related to phase with exchange components related to kinetics, "
			<< this->formula;
		P_INSTANCE_POINTER error_msg(oss.str().c_str(), CONTINUE);
		P_INSTANCE_POINTER input_error++;
		return;
	}
}
void
cxxExchComp::multiply(double extensive)
{
	//char * formula;
	//double moles;
	this->moles *= extensive;
	//cxxNameDouble formula_totals;
	//cxxNameDouble totals; 
	this->totals.multiply(extensive);
	//double la;
	//double charge_balance;
	this->charge_balance *= extensive;
	//char   *phase_name;
	//double phase_proportion;  
	this->phase_proportion *= extensive;
	//double formula_z;
}

#ifdef USE_MPI
void
cxxExchComp::mpi_pack(std::vector < int >&ints,
					  std::vector < double >&doubles)
{
	extern cxxDictionary dictionary;
	ints.push_back(dictionary.string2int(this->formula));
	doubles.push_back(this->moles);
	this->formula_totals.mpi_pack(ints, doubles);
	this->totals.mpi_pack(ints, doubles);
	doubles.push_back(this->la);
	doubles.push_back(this->charge_balance);
	ints.push_back(dictionary.string2int(this->phase_name));
	doubles.push_back(this->phase_proportion);
	ints.push_back(dictionary.string2int(this->rate_name));
	doubles.push_back(this->formula_z);
}

void
cxxExchComp::mpi_unpack(int *ints, int *ii, double *doubles, int *dd)
{
	extern cxxDictionary dictionary;
	int i = *ii;
	int d = *dd;
	this->formula = dictionary.int2stdstring(ints[i++]);
	this->moles = doubles[d++];
	this->formula_totals.mpi_unpack(ints, &i, doubles, &d);
	this->totals.mpi_unpack(ints, &i, doubles, &d);
	this->la = doubles[d++];
	this->charge_balance = doubles[d++];
	this->phase_name = dictionary.int2stdstring(ints[i++]);
	this->phase_proportion = doubles[d++];
	this->rate_name = dictionary.int2stdstring(ints[i++]);
	this->formula_z = doubles[d++];
	*ii = i;
	*dd = d;
}
#endif
