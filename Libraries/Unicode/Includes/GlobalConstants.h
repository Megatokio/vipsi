#define U_SUC_UCS4              3               /* SimpleCasing.vs: sizeof simple uppercase table for UCS4 */
#define U_SUC_UCS4_START        0x010428        /* SimpleCasing.vs: first item of interest in simple uc table for UCS4 */
#define U_SUC_UCS4_END          0x010450        /* SimpleCasing.vs: last (excl.) item of interest in simple uc table for UCS4 */
#define U_SLC_UCS4              3               /* SimpleCasing.vs: sizeof simple lowercase table for UCS4 */
#define U_SLC_UCS4_START        0x010400        /* SimpleCasing.vs: first item of interest in simple lc table for UCS4 */
#define U_SLC_UCS4_END          0x010428        /* SimpleCasing.vs: last (excl.) item of interest in simple lc table for UCS4 */
#define U_SUC_EVEN_UCS2         119             /* SimpleCasing.vs: sizeof simple uppercase table for even UCS2 code points */
#define U_SUC_EVEN_UCS2_START   0x0062          /* SimpleCasing.vs: first item of interest in simple uc table for even UCS2 */
#define U_SUC_EVEN_UCS2_END     0xFF5C          /* SimpleCasing.vs: last (excl.) item of interest in simple uc table for even UCS2 */
#define U_SLC_EVEN_UCS2         135             /* SimpleCasing.vs: sizeof simple lowercase table for even UCS2 code points */
#define U_SLC_EVEN_UCS2_START   0x0042          /* SimpleCasing.vs: first item of interest in simple lc table for even UCS2 */
#define U_SLC_EVEN_UCS2_END     0xFF3C          /* SimpleCasing.vs: last (excl.) item of interest in simple lc table for even UCS2 */
#define U_SUC_ODD_UCS2          152             /* SimpleCasing.vs: sizeof simple uppercase table for odd UCS2 code points */
#define U_SUC_ODD_UCS2_START    0x0061          /* SimpleCasing.vs: first item of interest in simple uc table for odd UCS2 */
#define U_SUC_ODD_UCS2_END      0xFF5B          /* SimpleCasing.vs: last (excl.) item of interest in simple uc table for odd UCS2 */
#define U_SLC_ODD_UCS2          109             /* SimpleCasing.vs: sizeof simple lowercase table for odd UCS2 code points */
#define U_SLC_ODD_UCS2_START    0x0041          /* SimpleCasing.vs: first item of interest in simple lc table for odd UCS2 */
#define U_SLC_ODD_UCS2_END      0xFF3B          /* SimpleCasing.vs: last (excl.) item of interest in simple lc table for odd UCS2 */
#define U_STC_UCS2              12              /* SimpleCasing.vs: sizeof simple titlecase exception table for UCS2 */
#define U_STC_UCS2_START        0x01C4          /* SimpleCasing.vs: first item of interest in simple tc exception table for UCS2 */
#define U_STC_UCS2_END          0x01F3          /* SimpleCasing.vs: last (excl.) item of interest in simple tc exception table for UCS2 */
#define U_STC_UCS2_TWELVECASES  1               /* SimpleCasing.vs: simple titlecase: only 12 exceptions for UCS2 */
#define U_SC_END                0xFB18          /* SpecialCasing.vs: last (excl.) code point for special casing */
#define U_SC_START              0x0049          /* SpecialCasing.vs: first code point which needs special handling for full casing */
#define U_CCC_UCS2              253             /* CanonicalCombiningClass.vs: size of Canonical Combining Class Table ccc2 */
#define U_CCC_UCS2_START        0x0300          /* CanonicalCombiningClass.vs: first Code Point of Interest in ccc2 table */
#define U_CCC_UCS2_END          0xFE24          /* CanonicalCombiningClass.vs: last (excl.) Code Point of Interest in ccc2 table */
#define U_CCC_UCS4              26              /* CanonicalCombiningClass.vs: size of Canonical Combining Class Table ccc4 */
#define U_CCC_UCS4_START        0x010A0D        /* CanonicalCombiningClass.vs: first Code Point of Interest in ccc4 table */
#define U_CCC_UCS4_END          0x01D245        /* CanonicalCombiningClass.vs: last (excl.) Code Point of Interest in ccc4 table */
#define U_ccc_defaultvalue      U_ccc_0         /* CanonicalCombiningClass.vs: Canonical Combining Class default Property Value */
#define U_BLOCKS_UCS1           2               /* Blocks.vs: size of ucs1->block subtable */
#define U_BLOCKS_UCS2           131             /* Blocks.vs: size of ucs2->block subtable */
#define U_BLOCKS_UCS4           169             /* Blocks.vs: size of ucs4->block table */
#define U_SCRIPTS_UCS1          14              /* Scripts.vs: size of ucs1->script subtable */
#define U_SCRIPTS_UCS2          689             /* Scripts.vs: size of ucs2->script table */
#define U_SCRIPTS_UCS4          76              /* Scripts.vs: size of ucs4->script table */
#define U_PROPERTYGROUPS        6               /* Properties.vs:  */
#define U_PROPERTIES            88              /* Properties.vs:  */
#define U_ccc_propertyvalues    256             /* Properties.vs:  */
#define U_age_propertyvalues    9               /* Properties.vs:  */
#define U_bc_propertyvalues     19              /* Properties.vs:  */
#define U_blk_propertyvalues    146             /* Properties.vs:  */
#define U_dt_propertyvalues     18              /* Properties.vs:  */
#define U_ea_propertyvalues     6               /* Properties.vs:  */
#define U_gc_propertyvalues     38              /* Properties.vs:  */
#define U_gcb_propertyvalues    10              /* Properties.vs:  */
#define U_hst_propertyvalues    6               /* Properties.vs:  */
#define U_jg_propertyvalues     54              /* Properties.vs:  */
#define U_jt_propertyvalues     6               /* Properties.vs:  */
#define U_lb_propertyvalues     36              /* Properties.vs:  */
#define U_nfc_qc_propertyvalues 3               /* Properties.vs:  */
#define U_nfd_qc_propertyvalues 2               /* Properties.vs:  */
#define U_nfkc_qc_propertyvalues 3               /* Properties.vs:  */
#define U_nfkd_qc_propertyvalues 2               /* Properties.vs:  */
#define U_nt_propertyvalues     4               /* Properties.vs:  */
#define U_sc_propertyvalues     62              /* Properties.vs:  */
#define U_sb_propertyvalues     11              /* Properties.vs:  */
#define U_wb_propertyvalues     8               /* Properties.vs:  */
#define U_PROPERTYVALUES        699             /* Properties.vs:  */
#define U_CODEPOINTS_UCS2       13076           /* Names.vs: count of ucs2 code points defined in UnicodeData.txt */
#define U_CODEPOINTS_UCS4       3275            /* Names.vs: count of ucs4 code points defined in UnicodeData.txt */
#define U_CODEPOINTRANGES_UCS2  362             /* Names.vs: sizeof contiguous ucs2 code point ranges table */
#define U_CODEPOINTRANGES_UCS4  68              /* Names.vs: sizeof contiguous ucs4 code point ranges table */
#define U_NAMEARRAY_SIZE        451119          /* Names.vs: sizeof uncompressed Name array */
#define U_NAMEARRAY_COMPRESSED  84979           /* Names.vs: sizeof compressed Name array */
#define U_GC_UCS1               60              /* GeneralCategory.vs: sizeof ucs1->general category subtable */
#define U_GC_UCS2               1464            /* GeneralCategory.vs: sizeof ucs2->general category table */
#define U_GC_UCS2_START         0x0000          /* GeneralCategory.vs: first Code Point of Interest in gc table */
#define U_GC_UCS2_END           0xFFFE          /* GeneralCategory.vs: last (excl.) Code Point of Interest in gc table */
#define U_GC_UCS4               204             /* GeneralCategory.vs: sizeof ucs4->general category table */
#define U_GC_UCS4_START         0x010000        /* GeneralCategory.vs: first Code Point of Interest in ucs4 gc table */
#define U_GC_UCS4_END           0x10FFFE        /* GeneralCategory.vs: last (excl.) Code Point of Interest in ucs4 gc table */
#define U_gc_defaultvalue       U_gc_cn         /* GeneralCategory.vs: General Category default Property Value */
#define U_NUMERIC_UCS2          96              /* NumericValue.vs: sizeof ucs2 numeric values table */
#define U_NUMERIC_UCS4          80              /* NumericValue.vs: sizeof ucs4 numeric values table */
#define U_NUMERIC_UCS1          6               /* NumericValue.vs: sizeof ucs1 numeric values subtable */
#define U_EA_UCS1               25              /* EastAsianWidth.vs: size of East Asian Width ucs1 subtable */
#define U_EA_UCS2               202             /* EastAsianWidth.vs: size of East Asian Width ucs2 table */
#define U_EA_UCS4               5               /* EastAsianWidth.vs: size of East Asian Width ccc4 table */
#define U_ea_defaultvalue       U_ea_n          /* EastAsianWidth.vs: East Asian Width default Property Value */
#define U_EA_UCS1_START         0x0020          /* EastAsianWidth.vs: first non-dflt in East Asian Width ucs1 subtable */
#define U_EA_UCS1_END           0x00FE          /* EastAsianWidth.vs: last non-dflt in East Asian Width ucs1 subtable */
#define U_EA_UCS2_START         0x0101          /* EastAsianWidth.vs: first non-dflt in East Asian Width ucs2 table */
#define U_EA_UCS2_END           0xFFFD          /* EastAsianWidth.vs: last non-dflt in East Asian Width ucs2 table */
#define U_EA_UCS4_START         0x020000        /* EastAsianWidth.vs: first non-dflt in East Asian Width ucs4 table */
#define U_EA_UCS4_END           0x10FFFD        /* EastAsianWidth.vs: last non-dflt in East Asian Width ucs4 table */
