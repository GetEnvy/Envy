//
// GenreMap.cs
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008 and Shareaza 2002-2008
//
// Envy is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Shareaza is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// (http://www.gnu.org/licenses/gpl.html)

#region Using directives
using System;
using System.Collections;
using System.Text;

#endregion

namespace Schemas
{
	internal static class GenreMap
	{
		static Hashtable _map = new Hashtable();
		static GenreMap()
		{
			_map.Add(genreType.adv_animal, EnvyBook.PeerGenreType.OutdoorsNature);
			_map.Add(genreType.adv_geo, EnvyBook.PeerGenreType.Travel);
			_map.Add(genreType.adv_history, EnvyBook.PeerGenreType.History);
			_map.Add(genreType.adv_indian, EnvyBook.PeerGenreType.Nonfiction);
			_map.Add(genreType.adv_maritime, EnvyBook.PeerGenreType.Travel);
			_map.Add(genreType.adv_western, EnvyBook.PeerGenreType.Nonfiction);
			_map.Add(genreType.adventure, EnvyBook.PeerGenreType.Nonfiction);
			_map.Add(genreType.antique, EnvyBook.PeerGenreType.LiteratureFiction);
			_map.Add(genreType.antique_ant, EnvyBook.PeerGenreType.LiteratureFiction);
			_map.Add(genreType.antique_east, EnvyBook.PeerGenreType.LiteratureFiction);
			_map.Add(genreType.antique_european, EnvyBook.PeerGenreType.LiteratureFiction);
			_map.Add(genreType.antique_myths, EnvyBook.PeerGenreType.LiteratureFiction);
			_map.Add(genreType.antique_russian, EnvyBook.PeerGenreType.LiteratureFiction);
			_map.Add(genreType.child_adv, EnvyBook.PeerGenreType.ChildrensBooks);
			_map.Add(genreType.child_det, EnvyBook.PeerGenreType.ChildrensBooks);
			_map.Add(genreType.child_education, EnvyBook.PeerGenreType.ChildrensBooks);
			_map.Add(genreType.child_prose, EnvyBook.PeerGenreType.ChildrensBooks);
			_map.Add(genreType.child_sf, EnvyBook.PeerGenreType.ScienceFictionFantasy);
			_map.Add(genreType.child_tale, EnvyBook.PeerGenreType.ChildrensBooks);
			_map.Add(genreType.child_verse, EnvyBook.PeerGenreType.ChildrensBooks);
			_map.Add(genreType.children, EnvyBook.PeerGenreType.ChildrensBooks);
			_map.Add(genreType.comp_db, EnvyBook.PeerGenreType.ComputersInternet);
			_map.Add(genreType.comp_hard, EnvyBook.PeerGenreType.ComputersInternet);
			_map.Add(genreType.comp_osnet, EnvyBook.PeerGenreType.ComputersInternet);
			_map.Add(genreType.comp_programming, EnvyBook.PeerGenreType.ComputersInternet);
			_map.Add(genreType.comp_soft, EnvyBook.PeerGenreType.ComputersInternet);
			_map.Add(genreType.comp_www, EnvyBook.PeerGenreType.ComputersInternet);
			_map.Add(genreType.computers, EnvyBook.PeerGenreType.ComputersInternet);
			_map.Add(genreType.design, EnvyBook.PeerGenreType.ProfessionalTechnical);
			_map.Add(genreType.det_action, EnvyBook.PeerGenreType.MysteryThrillers);
			_map.Add(genreType.det_classic, EnvyBook.PeerGenreType.LiteratureFiction);
			_map.Add(genreType.det_crime, EnvyBook.PeerGenreType.MysteryThrillers);
			_map.Add(genreType.det_espionage, EnvyBook.PeerGenreType.Nonfiction);
			_map.Add(genreType.det_hard, EnvyBook.PeerGenreType.MysteryThrillers);
			_map.Add(genreType.det_history, EnvyBook.PeerGenreType.History);
			_map.Add(genreType.det_irony, EnvyBook.PeerGenreType.LiteratureFiction);
			_map.Add(genreType.det_maniac, EnvyBook.PeerGenreType.MysteryThrillers);
			_map.Add(genreType.det_police, EnvyBook.PeerGenreType.MysteryThrillers);
			_map.Add(genreType.det_political, EnvyBook.PeerGenreType.MysteryThrillers);
			_map.Add(genreType.detective, EnvyBook.PeerGenreType.LiteratureFiction);
			_map.Add(genreType.dramaturgy, EnvyBook.PeerGenreType.LiteratureFiction);
			_map.Add(genreType.home, EnvyBook.PeerGenreType.HomeGarden);
			_map.Add(genreType.home_cooking, EnvyBook.PeerGenreType.CookingFoodWine);
			_map.Add(genreType.home_crafts, EnvyBook.PeerGenreType.HomeGarden);
			_map.Add(genreType.home_diy, EnvyBook.PeerGenreType.HomeGarden);
			_map.Add(genreType.home_entertain, EnvyBook.PeerGenreType.Entertainment);
			_map.Add(genreType.home_garden, EnvyBook.PeerGenreType.HomeGarden);
			_map.Add(genreType.home_health, EnvyBook.PeerGenreType.HealthFitness);
			_map.Add(genreType.home_pets, EnvyBook.PeerGenreType.HomeGarden);
			_map.Add(genreType.home_sex, EnvyBook.PeerGenreType.HealthFitness);
			_map.Add(genreType.home_sport, EnvyBook.PeerGenreType.Sports);
			_map.Add(genreType.humor, EnvyBook.PeerGenreType.Comics);
			_map.Add(genreType.humor_anecdote, EnvyBook.PeerGenreType.Comics);
			_map.Add(genreType.humor_prose, EnvyBook.PeerGenreType.Comics);
			_map.Add(genreType.humor_verse, EnvyBook.PeerGenreType.LiteratureFiction);
			_map.Add(genreType.love_contemporary, EnvyBook.PeerGenreType.Romance);
			_map.Add(genreType.love_detective, EnvyBook.PeerGenreType.MysteryThrillers);
			_map.Add(genreType.love_erotica, EnvyBook.PeerGenreType.Romance);
			_map.Add(genreType.love_history, EnvyBook.PeerGenreType.History);
			_map.Add(genreType.love_short, EnvyBook.PeerGenreType.Romance);
			_map.Add(genreType.nonf_biography, EnvyBook.PeerGenreType.BiographiesMemoirs);
			_map.Add(genreType.nonf_criticism, EnvyBook.PeerGenreType.Nonfiction);
			_map.Add(genreType.nonf_publicism, EnvyBook.PeerGenreType.Nonfiction);
			_map.Add(genreType.nonfiction, EnvyBook.PeerGenreType.Nonfiction);
			_map.Add(genreType.poetry, EnvyBook.PeerGenreType.LiteratureFiction);
			_map.Add(genreType.prose_classic, EnvyBook.PeerGenreType.LiteratureFiction);
			_map.Add(genreType.prose_contemporary, EnvyBook.PeerGenreType.LiteratureFiction);
			_map.Add(genreType.prose_counter, EnvyBook.PeerGenreType.LiteratureFiction);
			_map.Add(genreType.prose_history, EnvyBook.PeerGenreType.History);
			_map.Add(genreType.prose_rus_classic, EnvyBook.PeerGenreType.LiteratureFiction);
			_map.Add(genreType.prose_su_classics, EnvyBook.PeerGenreType.LiteratureFiction);
			_map.Add(genreType.ref_dict, EnvyBook.PeerGenreType.Reference);
			_map.Add(genreType.ref_encyc, EnvyBook.PeerGenreType.Reference);
			_map.Add(genreType.ref_guide, EnvyBook.PeerGenreType.Reference);
			_map.Add(genreType.ref_ref, EnvyBook.PeerGenreType.Reference);
			_map.Add(genreType.reference, EnvyBook.PeerGenreType.Reference);
			_map.Add(genreType.religion, EnvyBook.PeerGenreType.ReligionSpirituality);
			_map.Add(genreType.religion_esoterics, EnvyBook.PeerGenreType.ReligionSpirituality);
			_map.Add(genreType.religion_rel, EnvyBook.PeerGenreType.ReligionSpirituality);
			_map.Add(genreType.religion_self, EnvyBook.PeerGenreType.MindBody);
			_map.Add(genreType.sci_biology, EnvyBook.PeerGenreType.Science);
			_map.Add(genreType.sci_business, EnvyBook.PeerGenreType.BusinessInvesting);
			_map.Add(genreType.sci_chem, EnvyBook.PeerGenreType.Science);
			_map.Add(genreType.sci_culture, EnvyBook.PeerGenreType.Science);
			_map.Add(genreType.sci_history, EnvyBook.PeerGenreType.History);
			_map.Add(genreType.sci_juris, EnvyBook.PeerGenreType.ProfessionalTechnical);
			_map.Add(genreType.sci_linguistic, EnvyBook.PeerGenreType.ProfessionalTechnical);
			_map.Add(genreType.sci_math, EnvyBook.PeerGenreType.ProfessionalTechnical);
			_map.Add(genreType.sci_medicine, EnvyBook.PeerGenreType.ProfessionalTechnical);
			_map.Add(genreType.sci_philosophy, EnvyBook.PeerGenreType.ProfessionalTechnical);
			_map.Add(genreType.sci_phys, EnvyBook.PeerGenreType.ProfessionalTechnical);
			_map.Add(genreType.sci_politics, EnvyBook.PeerGenreType.ProfessionalTechnical);
			_map.Add(genreType.sci_psychology, EnvyBook.PeerGenreType.ProfessionalTechnical);
			_map.Add(genreType.sci_religion, EnvyBook.PeerGenreType.ReligionSpirituality);
			_map.Add(genreType.sci_tech, EnvyBook.PeerGenreType.ProfessionalTechnical);
			_map.Add(genreType.science, EnvyBook.PeerGenreType.Science);
			_map.Add(genreType.sf, EnvyBook.PeerGenreType.ScienceFictionFantasy);
			_map.Add(genreType.sf_action, EnvyBook.PeerGenreType.ScienceFictionFantasy);
			_map.Add(genreType.sf_cyberpunk, EnvyBook.PeerGenreType.ScienceFictionFantasy);
			_map.Add(genreType.sf_detective, EnvyBook.PeerGenreType.ScienceFictionFantasy);
			_map.Add(genreType.sf_epic, EnvyBook.PeerGenreType.ScienceFictionFantasy);
			_map.Add(genreType.sf_fantasy, EnvyBook.PeerGenreType.ScienceFictionFantasy);
			_map.Add(genreType.sf_heroic, EnvyBook.PeerGenreType.ScienceFictionFantasy);
			_map.Add(genreType.sf_history, EnvyBook.PeerGenreType.ScienceFictionFantasy);
			_map.Add(genreType.sf_horror, EnvyBook.PeerGenreType.Horror);
			_map.Add(genreType.sf_humor, EnvyBook.PeerGenreType.Comics);
			_map.Add(genreType.sf_social, EnvyBook.PeerGenreType.ScienceFictionFantasy);
			_map.Add(genreType.sf_space, EnvyBook.PeerGenreType.ScienceFictionFantasy);
			_map.Add(genreType.thriller, EnvyBook.PeerGenreType.MysteryThrillers);
	}

		public static EnvyBook.PeerGenreType GetPeerGenre(genreType fbGenre) {
			return (EnvyBook.PeerGenreType)_map[fbGenre];
		}
	}
}
