//
// FictionBook.cs
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
using System.ComponentModel;
using System.Xml.Serialization;

#endregion

namespace Schemas
{
    [XmlType(Namespace = "http://www.gribuser.ru/xml/fictionbook/2.0")]
    [XmlRoot(Namespace = "http://www.gribuser.ru/xml/fictionbook/2.0", IsNullable = false)]
	public class FictionBook
	{
        [XmlElement("description")]
		public FictionBookDescription description;
	}

	[XmlType()]
	public class FictionBookDescription
	{
		// Generic information about the book
		[XmlElement("title-info")]
		public FictionBookDescriptionTitleinfo titleinfo;

		// Information about this particular (xml) document
		[XmlElement("document-info")]
		public FictionBookDescriptionDocumentinfo documentinfo;

		// Information about some paper/other published document,
		// that was used as a source of this xml document
		[XmlElement("publish-info")]
		public FictionBookDescriptionPublishinfo publishinfo;

		// Any other information about the book/document that didn't fit in the above groups
		[XmlElement("custom-info")]
		public FictionBookDescriptionCustominfo[] custominfo;
	}

	[XmlType()]
	public class FictionBookDescriptionTitleinfo
	{
		// Genre of this book, with the optional match percentage
		[XmlElement()]
		public FictionBookDescriptionTitleinfoGenre[] genre;

		// Author(s) of this book
		[XmlElement()]
		public FictionBookDescriptionTitleinfoAuthor[] author;

		// Book title
		[XmlElement("book-title")]
		public TextFieldType booktitle;

		// Annotation for this book
        [XmlElement()]
		public AnnotationType annotation;

		// Any keywords for this book, intended for use in search engines
        [XmlElement()]
		public TextFieldType keywords;

		// Date this book was written, can be inexact (2001-2002).
		// If an optional attribute is present, then it should contain some computer-readable
		// date from the interval for use by search and indexing engines
        [XmlElement()]
		public DateType date;

		[XmlArrayItem(ElementName="image", IsNullable=false)]
		public ImageType[] coverpage;

		[XmlElement(DataType="language")]
		public string lang;

		// Book source language if translation
		[XmlElement("src-lang", DataType="language")]
		public string srclang;

		// Translators if this is a translation
		[XmlElement()]
		public AuthorType[] translator;

		// Any sequences this book might be part of
		[XmlElement()]
		public SequenceType[] sequence;
	}

	[XmlType()]
	public class FictionBookDescriptionTitleinfoGenre
	{
		[DefaultValue("100")]
		[XmlAttribute(Namespace="", DataType="integer")]
		public string match = "100";

		[XmlText()]
		public genreType Value;
	}

	[XmlType()]
	public enum genreType
	{
		// SciFi, Fantasy
		sf_history,			// Alternative history
		sf_action,
		sf_epic,
		sf_heroic,
		sf_detective,
		sf_cyberpunk,
		sf_space,
		sf_social,
		sf_horror,			// Horror & Mystic
		sf_humor,
		sf_fantasy,
		sf,					// Science Fiction
		// Detectives, Thrillers
		det_classic,		// Classical Detective
		det_police,			// Police Stories
		det_action,
		det_irony,			// Ironical Detective
		det_history,		// Historical Detective
		det_espionage,
		det_crime,
		det_political,
		det_maniac,			// Maniacs
		det_hard,			// Hard-boiled Detective
		thriller,
		detective,
		// Prose
		prose_classic,
		prose_history,
		prose_contemporary,
		prose_counter,		// Counterculture
		prose_rus_classic,	// Russian Classics
		prose_su_classics,	// Soviet Classics
		// Romance
		love_contemporary,
		love_history,
		love_detective,
		love_short,
		love_erotica,
		// Adventure
		adv_western,
		adv_history,
		adv_indian,
		adv_maritime,
		adv_geo,			// Travel & Geography
		adv_animal,			// Nature & Animals
		adventure,
		// Children's
		child_tale,			// Fairy Tales
		child_verse,		// Verses
		child_prose,		// Prose for Kids
		child_sf,			// Science Fiction for Kids
		child_det,			// Detectives & Thrillers
		child_adv,			// Adventures for Kids
		child_education,	// Education for Kids
		children,			// For Kids: Miscellanious
		poetry,
		dramaturgy,
		antique_ant,
		antique_european,
		antique_russian,
		antique_east,
		antique_myths,
		antique,
		sci_history,
		sci_psychology,
		sci_culture,
		sci_religion,
		sci_philosophy,
		sci_politics,
		sci_business,
		sci_juris,
		sci_linguistic,
		sci_medicine,
		sci_phys,
		sci_math,
		sci_chem,
		sci_biology,
		sci_tech,
		science,
		comp_www,
		comp_programming,
		comp_hard,
		comp_soft,
		comp_db,
		comp_osnet,
		computers,
		ref_encyc,
		ref_dict,
		ref_ref,
		ref_guide,
		reference,
		nonf_biography,
		nonf_publicism,
		nonf_criticism,
		nonfiction,
		design,
		religion_rel,
		religion_esoterics,
		religion_self,
		religion,
		humor_anecdote,
		humor_prose,
		humor_verse,
		humor,
		home_cooking,
		home_pets,
		home_crafts,
		home_entertain,
		home_health,
		home_garden,
		home_diy,
		home_sport,
		home_sex,
		home
	}

	[XmlType()]
	public class FictionBookDescriptionTitleinfoAuthor : AuthorType
	{
	}

	// Information about a single author
	[XmlType()]
	[XmlInclude(typeof(FictionBookDescriptionTitleinfoAuthor))]
	public class AuthorType
	{
		[XmlIgnore()]
        public AuthorDataChoice[] ItemsElementName;

		[XmlElement("first-name", Type=typeof(TextFieldType))]
		[XmlElement("middle-name", Type=typeof(TextFieldType))]
		[XmlElement("last-name", Type=typeof(TextFieldType))]
		[XmlElement("nickname", Type=typeof(TextFieldType))]
		[XmlElement("home-page", Type=typeof(string))]
		[XmlElement("email", Type=typeof(string))]
		[XmlChoiceIdentifier("ItemsElementName")]
		public object[] Items;
	}

	[XmlType()]
	[XmlInclude(typeof(FictionBookDescriptionCustominfo))]
	public class TextFieldType
	{
		[XmlAttribute(Namespace="http://www.w3.org/XML/1998/namespace")]
		public string lang;

		[XmlText()]
		public string Value;
	}

	[XmlType()]
	public class FictionBookDescriptionCustominfo : TextFieldType
	{
		[XmlAttribute("info-type", Namespace="")]
		public string infotype;
	}

	// A cut-down version of <section> used in annotations
	[XmlType()]
	public class AnnotationType
	{
		[XmlAttribute(Namespace="", DataType="ID")]
		public string id;

		[XmlAttribute(Namespace="http://www.w3.org/XML/1998/namespace")]
		public string lang;

		[XmlElement("p", Type=typeof(ParaType))]
		[XmlElement("poem", Type=typeof(PoemType))]
		[XmlElement("cite", Type=typeof(CiteType))]
		[XmlElement("empty-line")]
		public object[] Items;
	}

	// A basic paragraph, may include simple formatting inside
	[XmlType()]
	public class ParaType : StyleType
	{
		[XmlAttribute(Namespace="", DataType="ID")]
		public string id;

		[XmlAttribute(Namespace="")]
		public string style;
	}

	// Markup
	[XmlType()]
	[XmlInclude(typeof(ParaType))]
	public class StyleType
	{
		[XmlAttribute(Namespace="http://www.w3.org/XML/1998/namespace")]
		public string lang;

		[XmlIgnore()]
		public StyleChoice2[] ItemsElementName;

		[XmlElement("strong", Type=typeof(StyleType))]
		[XmlElement("emphasis", Type=typeof(StyleType))]
		[XmlElement("style", Type=typeof(NamedStyleType))]
		[XmlElement("a", Type=typeof(LinkType))]
		[XmlElement("image", Type=typeof(ImageType))]
		[XmlChoiceIdentifier("ItemsElementName")]
		public object[] Items;

		[XmlText()]
		public string[] Text;
	}

	// Markup
	[XmlType()]
    [XmlInclude(typeof(StyleType))]
    public class NamedStyleType : StyleType
	{
		[XmlAttribute(Namespace="", DataType="token")]
		public string name;
	}

	// Generic hyperlinks. Cannot be nested. Footnotes should be implemented by links
	// referring to additional bodies in the same document
	[XmlType()]
	public class LinkType
	{
		[XmlAttribute(Namespace="http://www.w3.org/1999/xlink")]
		public string type;

		[XmlAttribute(Namespace="http://www.w3.org/1999/xlink")]
		public string href;

		[XmlAttribute("type", Namespace="", DataType="token")]
		public string type1;

		[XmlIgnore()]
        public StyleChoice1[] ItemsElementName;

		[XmlElement("strong")]
		[XmlElement("emphasis")]
		[XmlElement("style")]
		[XmlChoiceIdentifier("ItemsElementName")]
		public StyleLinkType[] Items;

		[XmlText()]
		public string[] Text;
	}

    [XmlType("StyleChoice1", IncludeInSchema = false)]
	public enum StyleChoice1
	{
		strong,
		emphasis,
		style
	}

    [XmlType("StyleChoice2", IncludeInSchema = false)]
    public enum StyleChoice2
    {
        strong,
        emphasis,
        style,
        a,
        image
    }

    [XmlType("AuthorDataChoice", IncludeInSchema = false)]
    public enum AuthorDataChoice
    {
        [XmlEnum("first-name")]
        firstname,
        [XmlEnum("middle-name")]
        middlename,
        [XmlEnum("last-name")]
        lastname,
        nickname,
        [XmlEnum("home-page")]
        homepage,
        email
    }

	// Markup
	[XmlType()]
	public class StyleLinkType
	{
		[XmlIgnore()]
		public ItemsChoiceType[] ItemsElementName;

		[XmlElement("strong")]
		[XmlElement("emphasis")]
		[XmlElement("style")]
		[XmlChoiceIdentifier("ItemsElementName")]
		public StyleLinkType[] Items;

		[XmlText()]
		public string[] Text;
	}

	[XmlType(IncludeInSchema=false)]
	public enum ItemsChoiceType
	{
		strong,
		emphasis,
		style
	}

	// An empty element with an image name as an attribute
	[XmlType()]
	public class ImageType
	{
		[XmlAttribute(Namespace="http://www.w3.org/1999/xlink")]
		public string type;

		[XmlAttribute(Namespace="http://www.w3.org/1999/xlink")]
		public string href;

		[XmlAttribute(Namespace="")]
		public string alt;
	}

	// A poem
	[XmlType()]
	public class PoemType
	{
		[XmlAttribute(Namespace="", DataType="ID")]
		public string id;

		[XmlAttribute(Namespace="http://www.w3.org/XML/1998/namespace")]
		public string lang;

		// Poem title
		public TitleType title;

		// Poem epigraph(s), if any
		[XmlElement()]
		public EpigraphType[] epigraph;

        // Each poem should have at least one stanza.
        // Stanzas are usually separated with empty lines by user agents.
        [XmlElement()]
		public PoemTypeStanza[] stanza;

		[XmlElement("text-author")]
		public TextFieldType[] textauthor;

		// Date this poem was written.
		public DateType date;
	}

	// A title, used in sections, poems and body elements
	[XmlType()]
	public class TitleType
	{
		[XmlAttribute(Namespace="http://www.w3.org/XML/1998/namespace")]
		public string lang;

        [XmlElement("p", Type = typeof(ParaType))]
        [XmlElement("empty-line")]
        public object[] Items;
	}

	// An epigraph
	[XmlType()]
	public class EpigraphType
	{
		[XmlAttribute(Namespace="", DataType="ID")]
		public string id;

		[XmlElement("p", Type=typeof(ParaType))]
		[XmlElement("poem", Type=typeof(PoemType))]
		[XmlElement("cite", Type=typeof(CiteType))]
		[XmlElement("empty-line")]
		public object[] Items;

		[XmlElement("text-author")]
		public TextFieldType[] textauthor;
	}

	// A citation with an optional citation author at the end
	[XmlType()]
	public class CiteType
	{
		[XmlAttribute(Namespace="", DataType="ID")]
		public string id;

		[XmlAttribute(Namespace="http://www.w3.org/XML/1998/namespace")]
		public string lang;

		[XmlElement("p", Type=typeof(ParaType))]
		[XmlElement("poem", Type=typeof(PoemType))]
		[XmlElement("empty-line")]
		public object[] Items;

		[XmlElement("text-author")]
		public TextFieldType[] textauthor;
	}

	[XmlType()]
	public class FictionBookBinary
	{
		[XmlAttribute("content-type", Namespace="")]
		public string ContentType;

		[XmlAttribute("id", Namespace="", DataType="ID")]
		public string Id;

		[XmlText(DataType = "base64Binary")]
		public byte[] Value;
	}

	[XmlType()]
	public class FictionBookDescriptionDocumentinfo
	{
		// Author(s) of this particular document
		[XmlElement()]
		public AuthorType[] author;

		// Any software used in preparation of this document, in free format
		[XmlElement("program-used")]
		public TextFieldType programused;

		// Date this document was created, same guidelines as in the <title-info> section apply
		public DateType date;

		// Source URL if this document is a conversion of some other (online) document
		[XmlElement("src-url")]
		public string[] srcurl;

		// Author of the original (online) document, if this is a conversion
		[XmlElement("src-ocr")]
		public TextFieldType srcocr;

		// This is a unique identifier for a document.
		// This must not change unless you make substantial updates to the document
		[XmlElement(DataType="token")]
		public string id;

		// Document version, in free format, should be incremented if the document is changed
		// and re-released to the public
		public System.Single version;

		public AnnotationType history;
	}

	// Human readable date, maybe not exact, with an optional computer readable variant
	[XmlType()]
	public class DateType
	{
		[XmlAttribute(Namespace="", DataType="date")]
		public System.DateTime value;

		[XmlIgnore()]
		public bool valueSpecified;

		[XmlAttribute(Namespace="http://www.w3.org/XML/1998/namespace")]
		public string lang;

		[XmlText()]
		public string Value;
	}

	[XmlType()]
	public class FictionBookDescriptionPublishinfo
	{
		// Original (paper) book name
		[XmlElement("book-name")]
		public TextFieldType bookname;

		// Original (paper) book publisher
		public TextFieldType publisher;

		// City where the original (paper) book was published
		public TextFieldType city;

		// Year of the original (paper) publication
		[XmlElement(DataType="gYear")]
		public string year;

		public TextFieldType isbn;

		[XmlElement()]
		public SequenceType[] sequence;
	}

	// Book sequences
    [XmlType()]
	public class SequenceType
	{
		[XmlAttribute()]
		public string name;

		[XmlAttribute(DataType="integer")]
		public string number;

		[XmlAttribute(Namespace="http://www.w3.org/XML/1998/namespace")]
		public string lang;

		[XmlElement()]
		public SequenceType[] sequence;
	}

	[XmlType()]
	public class PoemTypeStanza
	{
		[XmlAttribute(Namespace="http://www.w3.org/XML/1998/namespace")]
		public string lang;
		public TitleType title;
		public ParaType subtitle;

		// An individual line in a stanza
		[XmlElement()]
		public ParaType[] v;
	}
}
