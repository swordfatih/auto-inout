/////////////////////////////////////////////////
#include <iostream>
#include <vector>
#include <cstdint>
#include <fstream>
#include <random>
#include <chrono>
#include <string>

/////////////////////////////////////////////////
template <typename Type>
struct Ensemble
{
	std::vector<Type> list;
	uint16_t max = 10;
};

/////////////////////////////////////////////////  
struct Specialite
{
	std::string nom; 
	uint16_t cout_horaire = 0;
};

/////////////////////////////////////////////////  
struct Travailleur
{
	std::string nom;
	std::vector<bool> tags_competences;
};

/////////////////////////////////////////////////  
struct Client
{
	std::string nom;
};

/////////////////////////////////////////////////  
struct Tache
{
	uint16_t nb_heures_requises;
	uint16_t nb_heures_effectuees;
	uint16_t indice_travailleur;
};
 
///////////////////////////////////////////////// 
struct Commande
{
	std::string nom;
	uint16_t idx_client = 0;
	std::vector<Tache> taches_par_specialite;
};

/////////////////////////////////////////////////
template <typename Type>
void fill(Ensemble<Type>& ensemble, std::string type_name)
{
	std::ifstream read("list/" + type_name + ".txt", std::ios::in);

	if (read)
	{
		for (uint16_t i = 0; i < ensemble.max; ++i)
		{
			Type type;
			read >> type.nom;

			ensemble.list.push_back({ type });
		}

		read.close();
	}
	else
	{
		std::cout << "couldn't open file : " << type_name << std::endl;
	}
}

////////////////////////////////////////////////////////////
static std::mt19937& random_generator()
{
	static std::mt19937 random_generator(std::chrono::high_resolution_clock::now().time_since_epoch().count());
	return random_generator;
}

/////////////////////////////////////////////////
void developpe(std::ofstream& in, Ensemble<Specialite>& specialites)
{
	std::uniform_int_distribution<std::mt19937::result_type> distribution_cout(1, 500);

	for (auto& it: specialites.list)
	{
		uint16_t cout_horaire = distribution_cout(random_generator());

		it.cout_horaire = cout_horaire;

		in << "developpe " << it.nom << ' ' << cout_horaire << std::endl;
	}
}

/////////////////////////////////////////////////
void embauche(std::ofstream& in, Ensemble<Specialite>& specialites, Ensemble<Travailleur>& travailleurs)
{
	std::uniform_int_distribution<std::mt19937::result_type> distribution_travailleurs_count(1, specialites.max);
	std::uniform_int_distribution<std::mt19937::result_type> distribution_travailleurs(0, travailleurs.max - 1);

	for (auto& it: travailleurs.list)
	{
		for (uint16_t i = 0; i < specialites.max; ++i)
		{
			it.tags_competences.push_back(false);
		}
	} 
	    
	for (uint16_t i = 0; i < specialites.max; ++i)
	{
		for (uint16_t j = 0; j < distribution_travailleurs_count(random_generator()); ++j)
		{
			uint16_t travailleur = distribution_travailleurs(random_generator());

			travailleurs.list[travailleur].tags_competences[i] = true;

			in << "embauche " << travailleurs.list[travailleur].nom << ' ' << specialites.list[i].nom << std::endl;
		}
	}
}

/////////////////////////////////////////////////
void demarche(std::ofstream& in, Ensemble<Client>& client)
{
	for (auto& it : client.list)
	{
		in << "demarche " << it.nom << std::endl;
	}
}

/////////////////////////////////////////////////
void commande(std::ofstream& in, Ensemble<Client>& clients, Ensemble<Commande>& commandes)
{
	std::uniform_int_distribution<std::mt19937::result_type> distribution_client(0, clients.max - 1);

	for (auto& it: commandes.list)
	{
		uint16_t client = distribution_client(random_generator());

		it.idx_client = client;

		in << "commande " << it.nom << ' ' << clients.list[client].nom << std::endl;
	}
}

/////////////////////////////////////////////////
void affect(Ensemble<Specialite>& specialites, Ensemble<Travailleur>& travailleurs, Ensemble<Commande>& commandes, uint16_t indice_commande, uint16_t indice_specialite)
{
	int16_t minimum = -1;
	uint16_t best_travailleur = 0; 

	for (uint16_t indice_travailleur = 0; indice_travailleur < travailleurs.max; ++indice_travailleur) 
	{
		if (travailleurs.list[indice_travailleur].tags_competences[indice_specialite] == true) 
		{
			int heure_total = 0;

			for (uint16_t indice_commande = 0; indice_commande < commandes.max; ++indice_commande)
			{
				for (uint16_t indice_specialite = 0; indice_specialite < specialites.max; ++indice_specialite) 
				{
					Tache* tache = &commandes.list[indice_commande].taches_par_specialite[indice_specialite];
					if (tache->indice_travailleur == indice_travailleur)
					{
						heure_total += tache->nb_heures_requises - tache->nb_heures_effectuees;
					}
				}
			}

			if (heure_total < minimum || minimum == -1) 
			{
				best_travailleur = indice_travailleur;
				minimum = heure_total; 
			}
		}
	}

	commandes.list[indice_commande].taches_par_specialite[indice_specialite].indice_travailleur = best_travailleur; 
}

/////////////////////////////////////////////////
void tache(std::ofstream& in, Ensemble<Specialite>& specialites, Ensemble<Travailleur>& travailleurs, Ensemble<Commande>& commandes)
{
	std::uniform_int_distribution<std::mt19937::result_type> distribution_specialite_count(1, specialites.max);
	std::uniform_int_distribution<std::mt19937::result_type> distribution_specialite(0, specialites.max - 1);
	std::uniform_int_distribution<std::mt19937::result_type> distribution_requise(1, 100);

	for (auto& it : commandes.list)
	{
		for (uint16_t i = 0; i < specialites.max; ++i)
		{
			it.taches_par_specialite.push_back({ 0, 0, 0 });
		}
	}

	for (uint16_t i = 0; i < commandes.max; ++i)
	{
		for (uint16_t j = 0; j < distribution_specialite_count(random_generator()); ++j)
		{
			uint16_t indice_specialite = distribution_specialite(random_generator());
			uint16_t heures_requises = distribution_requise(random_generator());

			if (commandes.list[i].taches_par_specialite[indice_specialite].nb_heures_requises == 0)
			{
				commandes.list[i].taches_par_specialite[indice_specialite].nb_heures_requises = heures_requises;

				affect(specialites, travailleurs, commandes, i, indice_specialite);

				in << "tache " << commandes.list[i].nom << ' ' << specialites.list[indice_specialite].nom << ' ' << heures_requises << std::endl;
			}
		}
	}
}

///////////////////////////////////////////////// 
bool commande_done(Ensemble<Specialite>& specialites, Ensemble<Commande>& commandes, uint16_t indice_commande)
{
	unsigned int indice_specialite;
	for (indice_specialite = 0; indice_specialite < specialites.max; ++indice_specialite)
	{
		auto&& tache = commandes.list[indice_commande].taches_par_specialite[indice_specialite];

		if (tache.nb_heures_effectuees < tache.nb_heures_requises)
		{
			return false;
		}
	}

	return true;
}

/////////////////////////////////////////////////
void print_facture(std::ofstream& out, Ensemble<Specialite>& specialites, Ensemble<Commande>& commandes, const unsigned int indice_commande)
{
	out << "facturation " << commandes.list[indice_commande].nom << " : ";

	bool first = true;

	unsigned int indice_specialite;
	for (indice_specialite = 0; indice_specialite < specialites.max; ++indice_specialite)
	{
		if (commandes.list[indice_commande].taches_par_specialite[indice_specialite].nb_heures_requises != 0)
		{
			if (first == true)
			{
				first = false;
			}
			else
			{
				out << ", ";
			}

			out << specialites.list[indice_specialite].nom
				<< ':'
				<< commandes.list[indice_commande].taches_par_specialite[indice_specialite].nb_heures_requises * specialites.list[indice_specialite].cout_horaire;
		}
	}

	out << std::endl;
}

/////////////////////////////////////////////////
void progression(std::ofstream& in, std::ofstream& out, Ensemble<Specialite>& specialites, Ensemble<Travailleur>& travailleurs, Ensemble<Commande>& commandes, Ensemble<Client>& clients)
{
	std::uniform_int_distribution<std::mt19937::result_type> distribution_progres(5, 30);
	std::uniform_int_distribution<std::mt19937::result_type> distribution_passe(1, 9);

	auto all_done = [&]()
	{
		// Puis on vérifie si toutes les commandes sont terminées
		for (uint16_t t = 0; t < commandes.max; ++t)
		{
			if (!commande_done(specialites, commandes, t))
			{
				// Une commande n'est pas terminée, on quitte la fonction
				return false;
			}
		}

		return true;
	};

	while(!all_done())
	{
		for (uint16_t j = 0; j < commandes.max; ++j)
		{ 
			for (uint16_t k = 0; k < specialites.max; ++k)
			{
				if (commandes.list[j].taches_par_specialite[k].nb_heures_requises != 0 && commandes.list[j].taches_par_specialite[k].nb_heures_effectuees < commandes.list[j].taches_par_specialite[k].nb_heures_requises)
				{
					uint16_t heures_effectuees = distribution_progres(random_generator());

					commandes.list[j].taches_par_specialite[k].nb_heures_effectuees += heures_effectuees;

					in << "progression " << commandes.list[j].nom << ' ' << specialites.list[k].nom << ' ' << heures_effectuees;

					if (distribution_passe(random_generator()) % 3 == 0)
					{
						affect(specialites, travailleurs, commandes, j, k);
						in << " passe";
					}

					in << std::endl;

					if (commandes.list[j].taches_par_specialite[k].nb_heures_effectuees >= commandes.list[j].taches_par_specialite[k].nb_heures_requises)
					{
						// On verifie si la commande en entier est terminée
						if (commande_done(specialites, commandes, j))
						{
							// Si elle l'est, on la facture
							print_facture(out, specialites, commandes, j);

							if (all_done())
							{
								// Si oui, on facture
								out << "facturations : ";

								for (uint16_t indice_client = 0; indice_client < clients.max; ++indice_client)
								{
									if (indice_client != 0)
									{
										out << ", ";
									}

									unsigned long total_client = 0;

									for (uint16_t t = 0; t < commandes.max; ++t)
									{
										if (commandes.list[t].idx_client == indice_client)
										{
											unsigned long total_commande = 0;

											unsigned int indice_specialite;
											for (indice_specialite = 0; indice_specialite < specialites.max; ++indice_specialite)
											{
												total_commande += commandes.list[t].taches_par_specialite[indice_specialite].nb_heures_requises * specialites.list[indice_specialite].cout_horaire;
											}

											total_client += total_commande;
										}
									}

									out << clients.list[indice_client].nom << ':' << total_client;
								}

								out << std::endl;

								return;
							}
						}
					}
				}
			}
		}
	}
}

/////////////////////////////////////////////////
int main()
{
	Ensemble<Specialite> specialites;
	Ensemble<Travailleur> travailleurs;
	Ensemble<Client> clients;
	Ensemble<Commande> commandes;

	std::ifstream config("config.txt");

	if (config)
	{
		std::vector<int32_t> max;

		for (uint16_t i = 0; i < 4; ++i)
		{
			std::string buffer;
			config >> buffer;

			max.push_back(std::stoi(buffer));
		}

		specialites.max = max[0];
		travailleurs.max = max[1];
		clients.max = max[2];
		commandes.max = max[3];

		config.close();
	}
	else
	{
		std::cout << "Il tient de signaler à sa Majesté le Roy de l'occurence inédite d'innatendu naufrage fonctionnel. (config manquant)" << std::endl;
	}

	fill(specialites, "specialites");
	fill(travailleurs, "travailleurs");
	fill(clients, "clients");
	fill(commandes, "commandes");

	std::ofstream in("in.txt");
	std::ofstream out("out.txt");

	if(in && out)
	{ 
		developpe(in, specialites);
		embauche(in, specialites, travailleurs);
		demarche(in, clients);
		commande(in, clients, commandes);
		tache(in, specialites, travailleurs, commandes);

		progression(in, out, specialites, travailleurs, commandes, clients);

		in.close();
		out.close();
	}
	else
	{
		std::cout << "Il tient de signaler à sa Majesté le Roy de l'occurence inédite d'innatendu naufrage fonctionnel. (in/out ne peut être crée)" << std::endl;
	}

	return 0;
}
