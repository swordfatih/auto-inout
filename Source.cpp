/////////////////////////////////////////////////
// Headers
/////////////////////////////////////////////////
#include <iostream>
#include <cstdint>
#include <fstream>
#include <cstdio>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>   

/////////////////////////////////////////////////
enum class Cardinal 
{
	NORD = 0,
	SUD
};

/////////////////////////////////////////////////
struct Base
{
	uint16_t asc;
	uint16_t desc;
	uint16_t pioche;
	uint16_t main_size;
	uint16_t pioche_count;
	uint16_t play_count;
	std::vector<int> main;
	bool first = true;
};

/////////////////////////////////////////////////
struct Game
{
	Base bases[2] = { {1, 60, 52, 6}, {1, 60, 52, 6} };
	Cardinal tour = Cardinal::NORD;
} game;

/////////////////////////////////////////////////
std::istream& expect(std::istream& in, const std::string& input)
{
	for (auto c : input)
	{
		if ((in >> std::ws).peek() == c)
		{
			in.ignore();
		}
		else
		{
			in.setstate(std::ios_base::failbit);
		}
	}

	return in;
}

/////////////////////////////////////////////////
bool check_values(Base& base, const std::string& input)
{
	int asc;
	int desc;
	int main;
	int pioche;

	std::istringstream stream(input);

	char token[2];

	expect(stream, "^[").read(&token[0], 2);
	asc = std::stoi(token);

	expect(stream, "]v[").read(&token[0], 2);
	desc = std::stoi(token);

	token[0] = '0';
	expect(stream, "](m").read(&token[1], 1);
	main = std::stoi(token);

	token[0] = '0';
	expect(stream, "p").read(&token[0], 2);
	pioche = std::stoi(token);

	bool value = asc == base.asc && desc == base.desc && ((base.first && main == 6) || main == base.main_size) && pioche == base.pioche;

	if (base.first)
	{
		base.first = false;
	}

	return value;
}

/////////////////////////////////////////////////
bool partie_finie();

/////////////////////////////////////////////////
bool check_cards(Base& base, const std::string& input)
{
	std::vector<int> cards;
	
	std::istringstream stream(input);

	expect(stream, "{");

	std::string token;
	while (stream >> token && token != "}")
	{
		cards.push_back(std::stoi(token));
	}

	if (!std::is_sorted(std::begin(cards), std::end(cards)))
	{
		return false;
	}

	if (base.main.empty()) 
	{
		for (auto card : cards)
		{
			base.main.push_back(card);
		}
	}
	else
	{
		uint16_t not_match_count = 0;

		for (uint16_t i = 0; i < cards.size(); ++i)
		{
			if (std::find(base.main.begin(), base.main.end(), cards[i]) == base.main.end())
			{
				not_match_count++;
				
				base.main.push_back(cards[i]);
				std::sort(base.main.begin(), base.main.end());
			}
		}

		if (not_match_count != base.pioche_count)
		{
			return false;
		}
	}

	if (partie_finie() && std::cin.peek() != 'p')
	{
		return false;
	}

	return true;
}

/////////////////////////////////////////////////
bool error_on_next_line()
{
	return std::cin.peek() == '#';
}

/////////////////////////////////////////////////
struct Coup
{
	uint16_t carte;
	enum class Sens { ASC, DESC } sens;
	bool adversaire;

	static std::vector<Coup> getCombinaisons(uint16_t carte)
	{
		std::vector<Coup> coups;

		for (int k = 0; k < 2; ++k)
		{
			Sens sens = Sens::ASC;

			if (k != 0)
			{
				sens = Sens::DESC;
			}

			for (int l = 0; l < 2; ++l)
			{
				bool adversaire = false;

				if (l != 0)
				{
					adversaire = true;
				}

				coups.push_back(Coup{ carte, sens, adversaire });
			}
		}

		return coups;
	}
};

/////////////////////////////////////////////////
bool check_play(const std::string& input)
{
	if((input.rfind("#>", 0) == 0 && input.size() <= 2) || (input.rfind(">", 0) == 0 && input.size() <= 1))
	{
		return error_on_next_line();
	}

	std::string token;
	std::istringstream stream(input);

	stream >> token;

	if (token != "#>" && token != ">")
	{
		return error_on_next_line();
	}

	std::vector<Coup> coups;
	while (stream >> token)
	{
		if (token.size() != 3 && token.size() != 4)
		{
			return error_on_next_line();
		}

		if (!isdigit(token[0]) || !isdigit(token[1]))
		{
			return error_on_next_line();
		}

		if (token[2] != 'v' && token[2] != '^')
		{
			return error_on_next_line();
		}

		if (token.size() == 4 && token[3] != '\'')
		{
			return error_on_next_line();
		}

		coups.push_back({ static_cast<uint16_t>(std::stoi(token.substr(0, 2))), token[2] == '^' ? Coup::Sens::ASC : Coup::Sens::DESC, token.size() == 4});
	}

	Game simulation = game;
	Base& simulation_tour = simulation.bases[static_cast<uint16_t>(simulation.tour)];

	Cardinal adverse = simulation.tour == Cardinal::NORD ? Cardinal::SUD : Cardinal::NORD;
	uint16_t adversaire_count = 0;

	if (coups.size() < 2)
	{
		return error_on_next_line();
	}

	for (uint16_t i = 0; i < coups.size(); ++i)
	{
		if (coups[i].carte < 1 || coups[i].carte > 60)
		{
			return error_on_next_line();
		}

		if (std::find(simulation_tour.main.begin(), simulation_tour.main.end(), coups[i].carte) == simulation_tour.main.end())
		{
			return error_on_next_line();
		}

		adversaire_count += coups[i].adversaire;

		Cardinal cardinal_to_play = coups[i].adversaire ? adverse : simulation.tour;
		Base& base_to_play = simulation.bases[static_cast<uint16_t>(cardinal_to_play)];

		if(coups[i].sens == Coup::Sens::ASC)
		{ 
			if (!coups[i].adversaire && coups[i].carte > base_to_play.asc || !coups[i].adversaire && coups[i].carte == base_to_play.asc - 10 || coups[i].adversaire && coups[i].carte < base_to_play.asc)
			{
				base_to_play.asc = coups[i].carte;
			}
			else
			{
				return error_on_next_line();
			}
		}
		else
		{
			if (!coups[i].adversaire && coups[i].carte < base_to_play.desc || !coups[i].adversaire && coups[i].carte == base_to_play.desc + 10 || coups[i].adversaire && coups[i].carte > base_to_play.desc)
			{
				base_to_play.desc = coups[i].carte;
			}
			else
			{
				return error_on_next_line();
			}
		}

		simulation_tour.main.erase(std::remove(simulation_tour.main.begin(), simulation_tour.main.end(), coups[i].carte), simulation_tour.main.end());
	}

	if (adversaire_count > 1)
	{
		return error_on_next_line();
	}

	simulation_tour.play_count = coups.size();

	simulation_tour.pioche_count = adversaire_count != 0 ? 6 - simulation_tour.main.size() : 2;

	if (simulation_tour.pioche < simulation_tour.pioche_count)
	{
		simulation_tour.pioche_count = simulation_tour.pioche;
	}

	simulation_tour.pioche -= simulation_tour.pioche_count;
	simulation_tour.main_size += simulation_tour.pioche_count - simulation_tour.play_count;

	simulation.tour = adverse;

	game = simulation;

	return true;
}

/////////////////////////////////////////////////
bool check_summary(const std::string& input)
{
	std::string token;
	std::istringstream stream(input);

	Cardinal played_cardinal = game.tour == Cardinal::NORD ? Cardinal::SUD : Cardinal::NORD;
	Base& played_base = game.bases[static_cast<int>(played_cardinal)];

	stream >> token;

	if (token.size() != 1 && std::stoi(token) != played_base.play_count)
	{
		return false;
	}

	stream >> token;

	if (token != "cartes")
	{
		return false;
	}

	stream >> token >> token;

	if (token.size() != 1 && std::stoi(token) != played_base.pioche_count)
	{
		return false;
	}

	stream >> token;

	if (token != "cartes")
	{
		return false;
	}

	return true;
}

/////////////////////////////////////////////////
bool hasValidMove(Base& source, Base& adverse)
{
	if (source.main.size() == 1)
	{
		return false;
	}

	for (int i = 0; i < source.main.size(); ++i)
	{
		for (int j = 0; j < source.main.size(); ++j)
		{
			if (i == j)
			{
				continue;
			}

			for (Coup first : Coup::getCombinaisons(source.main[i]))
			{
				for (Coup second : Coup::getCombinaisons(source.main[j]))
				{
					std::vector<Coup> coups;
					coups.push_back(first);
					coups.push_back(second);

					Base simulation_source = source;
					Base simulation_adverse = adverse;

					bool both_good = true;
					uint16_t adversaire_count = 0;

					for (uint16_t k = 0; k < coups.size(); ++k)
					{
						Base& base_to_play = coups[k].adversaire ? simulation_adverse : simulation_source;

						adversaire_count += coups[k].adversaire;

						if (coups[k].sens == Coup::Sens::ASC)
						{
							if (!coups[k].adversaire && coups[k].carte > base_to_play.asc || !coups[k].adversaire && coups[k].carte == base_to_play.asc - 10 || coups[k].adversaire && coups[k].carte < base_to_play.asc)
							{
								base_to_play.asc = coups[k].carte;
							}
							else
							{
								both_good = false;
							}
						}
						else
						{
							if (!coups[k].adversaire && coups[k].carte < base_to_play.desc || !coups[k].adversaire && coups[k].carte == base_to_play.desc + 10 || coups[k].adversaire && coups[k].carte > base_to_play.desc)
							{
								base_to_play.desc = coups[k].carte;
							}
							else
							{
								both_good = false;
							}
						}

						simulation_source.main.erase(std::remove(simulation_source.main.begin(), simulation_source.main.end(), coups[k].carte), simulation_source.main.end());
					}

					if (both_good && adversaire_count <= 1)
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

/////////////////////////////////////////////////
bool partie_finie()
{
	Base& courante = game.bases[static_cast<int>(game.tour)];
	Base& adverse = game.bases[static_cast<int>(game.tour == Cardinal::NORD ? Cardinal::SUD : Cardinal::NORD)];

	return !hasValidMove(courante, adverse) || (adverse.main_size == 0 && adverse.pioche == 0);
}

/////////////////////////////////////////////////
bool partie_finie(const std::string& input)
{
	Base& courante = game.bases[static_cast<int>(game.tour)];
	Base& adverse = game.bases[static_cast<int>(game.tour == Cardinal::NORD ? Cardinal::SUD : Cardinal::NORD)];

	if (!hasValidMove(courante, adverse) || (adverse.main_size == 0 && adverse.pioche == 0))
	{
		std::string token;
		std::istringstream stream(input);

		stream >> token >> token >> token;

		return token == "NORD" && game.tour == Cardinal::SUD || token == "SUD" && game.tour == Cardinal::NORD;
	}
	else
	{
		return false;
	}
}

/////////////////////////////////////////////////
bool parse(const std::string& token)
{
	if (token.size() < 1)
	{
		return false;
	}

	if (token.rfind("NORD", 0) == 0)
	{
		return check_values(game.bases[static_cast<uint16_t>(Cardinal::NORD)], token.substr(5));
	}
	else if (token.rfind("SUD", 0) == 0)
	{
		return check_values(game.bases[static_cast<uint16_t>(Cardinal::SUD)], token.substr(4));
	}
	else if (token.rfind("cartes NORD", 0) == 0)
	{
		return check_cards(game.bases[static_cast<uint16_t>(Cardinal::NORD)], token.substr(11));
	}
	else if (token.rfind("cartes SUD", 0) == 0)
	{
		return check_cards(game.bases[static_cast<uint16_t>(Cardinal::SUD)], token.substr(10));
	}
	else if (token.rfind(">", 0) == 0 || token.rfind("#>", 0) == 0)
	{
		return check_play(token);
	}
	else if (isdigit(token[0]))
	{
		return check_summary(token);
	}
	else if (token.rfind("partie", 0) == 0)
	{
		return partie_finie(token);
	}
	else
	{
		return false;
	}

	return true;
}

/////////////////////////////////////////////////
int main()
{
	std::string token;
	uint64_t count = 0;
	uint64_t error_count = 0;
	while (std::getline(std::cin, token) && token.rfind("fin", 0) != 0)
	{
		if (!parse(token))
		{
			std::cout << "Robocop triste. Votre code echoue sur '" << token << "' a la ligne " << count << " /:" << std::endl;
			error_count++;
		}

		count++;
	}

	if (error_count == 0)
	{
		std::cout << "Soit Robocop est nul. Soit vous etes parfait. Robocop pense que vous etes parfait. (" << error_count << " erreur trouve)" << std::endl;
	}
	else
	{
		std::cout << "Robocop pense qu'il est bidon, il a etonnament trouve " << error_count << " erreur(s) dans votre magnifique programme." << std::endl;
	}

	std::cout << "Robocop a ainsi termine d'examiner votre code." << std::endl;

	return 0;
}
