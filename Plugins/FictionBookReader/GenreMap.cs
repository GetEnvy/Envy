//
// GenreMap.cs
//
// This file is part of Envy (getenvy.com) © 2016-2018
// Portions copyright Shareaza 2002-2008 and PeerProject 2008
//
// Envy is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version (at your option).
//
// Shareaza is distributed in the hope that it will be useful,
// but AS-IS WITHOUT ANY WARRANTY; without even implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
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
			_map.Add(genreType.adv_animal, EnvyBook.EnvyGenreType.OutdoorsNature);
			_map.Add(genreType.adv_geo, EnvyBook.EnvyGenreType.Travel);
			_map.Add(genreType.adv_history, EnvyBook.EnvyGenreType.History);
			_map.Add(genreType.adv_indian, EnvyBook.EnvyGenreType.Nonfiction);
			_map.Add(genreType.adv_maritime, EnvyBook.EnvyGenreType.Travel);
			_map.Add(genreType.adv_western, EnvyBook.EnvyGenreType.Nonfiction);
			_map.Add(genreType.adventure, EnvyBook.EnvyGenreType.Nonfiction);
			_map.Add(genreType.antique, EnvyBook.EnvyGenreType.LiteratureFiction);
			_map.Add(genreType.antique_ant, EnvyBook.EnvyGenreType.LiteratureFiction);
			_map.Add(genreType.antique_east, EnvyBook.EnvyGenreType.LiteratureFiction);
			_map.Add(genreType.antique_european, EnvyBook.EnvyGenreType.LiteratureFiction);
			_map.Add(genreType.antique_myths, EnvyBook.EnvyGenreType.LiteratureFiction);
			_map.Add(genreType.antique_russian, EnvyBook.EnvyGenreType.LiteratureFiction);
			_map.Add(genreType.child_adv, EnvyBook.EnvyGenreType.ChildrensBooks);
			_map.Add(genreType.child_det, EnvyBook.EnvyGenreType.ChildrensBooks);
			_map.Add(genreType.child_education, EnvyBook.EnvyGenreType.ChildrensBooks);
			_map.Add(genreType.child_prose, EnvyBook.EnvyGenreType.ChildrensBooks);
			_map.Add(genreType.child_sf, EnvyBook.EnvyGenreType.ScienceFictionFantasy);
			_map.Add(genreType.child_tale, EnvyBook.EnvyGenreType.ChildrensBooks);
			_map.Add(genreType.child_verse, EnvyBook.EnvyGenreType.ChildrensBooks);
			_map.Add(genreType.children, EnvyBook.EnvyGenreType.ChildrensBooks);
			_map.Add(genreType.comp_db, EnvyBook.EnvyGenreType.ComputersInternet);
			_map.Add(genreType.comp_hard, EnvyBook.EnvyGenreType.ComputersInternet);
			_map.Add(genreType.comp_osnet, EnvyBook.EnvyGenreType.ComputersInternet);
			_map.Add(genreType.comp_programming, EnvyBook.EnvyGenreType.ComputersInternet);
			_map.Add(genreType.comp_soft, EnvyBook.EnvyGenreType.ComputersInternet);
			_map.Add(genreType.comp_www, EnvyBook.EnvyGenreType.ComputersInternet);
			_map.Add(genreType.computers, EnvyBook.EnvyGenreType.ComputersInternet);
			_map.Add(genreType.design, EnvyBook.EnvyGenreType.ProfessionalTechnical);
			_map.Add(genreType.det_action, EnvyBook.EnvyGenreType.MysteryThrillers);
			_map.Add(genreType.det_classic, EnvyBook.EnvyGenreType.LiteratureFiction);
			_map.Add(genreType.det_crime, EnvyBook.EnvyGenreType.MysteryThrillers);
			_map.Add(genreType.det_espionage, EnvyBook.EnvyGenreType.Nonfiction);
			_map.Add(genreType.det_hard, EnvyBook.EnvyGenreType.MysteryThrillers);
			_map.Add(genreType.det_history, EnvyBook.EnvyGenreType.History);
			_map.Add(genreType.det_irony, EnvyBook.EnvyGenreType.LiteratureFiction);
			_map.Add(genreType.det_maniac, EnvyBook.EnvyGenreType.MysteryThrillers);
			_map.Add(genreType.det_police, EnvyBook.EnvyGenreType.MysteryThrillers);
			_map.Add(genreType.det_political, EnvyBook.EnvyGenreType.MysteryThrillers);
			_map.Add(genreType.detective, EnvyBook.EnvyGenreType.LiteratureFiction);
			_map.Add(genreType.dramaturgy, EnvyBook.EnvyGenreType.LiteratureFiction);
			_map.Add(genreType.home, EnvyBook.EnvyGenreType.HomeGarden);
			_map.Add(genreType.home_cooking, EnvyBook.EnvyGenreType.CookingFoodWine);
			_map.Add(genreType.home_crafts, EnvyBook.EnvyGenreType.HomeGarden);
			_map.Add(genreType.home_diy, EnvyBook.EnvyGenreType.HomeGarden);
			_map.Add(genreType.home_entertain, EnvyBook.EnvyGenreType.Entertainment);
			_map.Add(genreType.home_garden, EnvyBook.EnvyGenreType.HomeGarden);
			_map.Add(genreType.home_health, EnvyBook.EnvyGenreType.HealthFitness);
			_map.Add(genreType.home_pets, EnvyBook.EnvyGenreType.HomeGarden);
			_map.Add(genreType.home_sex, EnvyBook.EnvyGenreType.HealthFitness);
			_map.Add(genreType.home_sport, EnvyBook.EnvyGenreType.Sports);
			_map.Add(genreType.humor, EnvyBook.EnvyGenreType.Comics);
			_map.Add(genreType.humor_anecdote, EnvyBook.EnvyGenreType.Comics);
			_map.Add(genreType.humor_prose, EnvyBook.EnvyGenreType.Comics);
			_map.Add(genreType.humor_verse, EnvyBook.EnvyGenreType.LiteratureFiction);
			_map.Add(genreType.love_contemporary, EnvyBook.EnvyGenreType.Romance);
			_map.Add(genreType.love_detective, EnvyBook.EnvyGenreType.MysteryThrillers);
			_map.Add(genreType.love_erotica, EnvyBook.EnvyGenreType.Romance);
			_map.Add(genreType.love_history, EnvyBook.EnvyGenreType.History);
			_map.Add(genreType.love_short, EnvyBook.EnvyGenreType.Romance);
			_map.Add(genreType.nonf_biography, EnvyBook.EnvyGenreType.BiographiesMemoirs);
			_map.Add(genreType.nonf_criticism, EnvyBook.EnvyGenreType.Nonfiction);
			_map.Add(genreType.nonf_publicism, EnvyBook.EnvyGenreType.Nonfiction);
			_map.Add(genreType.nonfiction, EnvyBook.EnvyGenreType.Nonfiction);
			_map.Add(genreType.poetry, EnvyBook.EnvyGenreType.LiteratureFiction);
			_map.Add(genreType.prose_classic, EnvyBook.EnvyGenreType.LiteratureFiction);
			_map.Add(genreType.prose_contemporary, EnvyBook.EnvyGenreType.LiteratureFiction);
			_map.Add(genreType.prose_counter, EnvyBook.EnvyGenreType.LiteratureFiction);
			_map.Add(genreType.prose_history, EnvyBook.EnvyGenreType.History);
			_map.Add(genreType.prose_rus_classic, EnvyBook.EnvyGenreType.LiteratureFiction);
			_map.Add(genreType.prose_su_classics, EnvyBook.EnvyGenreType.LiteratureFiction);
			_map.Add(genreType.ref_dict, EnvyBook.EnvyGenreType.Reference);
			_map.Add(genreType.ref_encyc, EnvyBook.EnvyGenreType.Reference);
			_map.Add(genreType.ref_guide, EnvyBook.EnvyGenreType.Reference);
			_map.Add(genreType.ref_ref, EnvyBook.EnvyGenreType.Reference);
			_map.Add(genreType.reference, EnvyBook.EnvyGenreType.Reference);
			_map.Add(genreType.religion, EnvyBook.EnvyGenreType.ReligionSpirituality);
			_map.Add(genreType.religion_esoterics, EnvyBook.EnvyGenreType.ReligionSpirituality);
			_map.Add(genreType.religion_rel, EnvyBook.EnvyGenreType.ReligionSpirituality);
			_map.Add(genreType.religion_self, EnvyBook.EnvyGenreType.MindBody);
			_map.Add(genreType.sci_biology, EnvyBook.EnvyGenreType.Science);
			_map.Add(genreType.sci_business, EnvyBook.EnvyGenreType.BusinessInvesting);
			_map.Add(genreType.sci_chem, EnvyBook.EnvyGenreType.Science);
			_map.Add(genreType.sci_culture, EnvyBook.EnvyGenreType.Science);
			_map.Add(genreType.sci_history, EnvyBook.EnvyGenreType.History);
			_map.Add(genreType.sci_juris, EnvyBook.EnvyGenreType.ProfessionalTechnical);
			_map.Add(genreType.sci_linguistic, EnvyBook.EnvyGenreType.ProfessionalTechnical);
			_map.Add(genreType.sci_math, EnvyBook.EnvyGenreType.ProfessionalTechnical);
			_map.Add(genreType.sci_medicine, EnvyBook.EnvyGenreType.ProfessionalTechnical);
			_map.Add(genreType.sci_philosophy, EnvyBook.EnvyGenreType.ProfessionalTechnical);
			_map.Add(genreType.sci_phys, EnvyBook.EnvyGenreType.ProfessionalTechnical);
			_map.Add(genreType.sci_politics, EnvyBook.EnvyGenreType.ProfessionalTechnical);
			_map.Add(genreType.sci_psychology, EnvyBook.EnvyGenreType.ProfessionalTechnical);
			_map.Add(genreType.sci_religion, EnvyBook.EnvyGenreType.ReligionSpirituality);
			_map.Add(genreType.sci_tech, EnvyBook.EnvyGenreType.ProfessionalTechnical);
			_map.Add(genreType.science, EnvyBook.EnvyGenreType.Science);
			_map.Add(genreType.sf, EnvyBook.EnvyGenreType.ScienceFictionFantasy);
			_map.Add(genreType.sf_action, EnvyBook.EnvyGenreType.ScienceFictionFantasy);
			_map.Add(genreType.sf_cyberpunk, EnvyBook.EnvyGenreType.ScienceFictionFantasy);
			_map.Add(genreType.sf_detective, EnvyBook.EnvyGenreType.ScienceFictionFantasy);
			_map.Add(genreType.sf_epic, EnvyBook.EnvyGenreType.ScienceFictionFantasy);
			_map.Add(genreType.sf_fantasy, EnvyBook.EnvyGenreType.ScienceFictionFantasy);
			_map.Add(genreType.sf_heroic, EnvyBook.EnvyGenreType.ScienceFictionFantasy);
			_map.Add(genreType.sf_history, EnvyBook.EnvyGenreType.ScienceFictionFantasy);
			_map.Add(genreType.sf_horror, EnvyBook.EnvyGenreType.Horror);
			_map.Add(genreType.sf_humor, EnvyBook.EnvyGenreType.Comics);
			_map.Add(genreType.sf_social, EnvyBook.EnvyGenreType.ScienceFictionFantasy);
			_map.Add(genreType.sf_space, EnvyBook.EnvyGenreType.ScienceFictionFantasy);
			_map.Add(genreType.thriller, EnvyBook.EnvyGenreType.MysteryThrillers);
	}

		public static EnvyBook.EnvyGenreType GetEnvyGenre(genreType fbGenre) {
			return (EnvyBook.EnvyGenreType)_map[fbGenre];
		}
	}
}
