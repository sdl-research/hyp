









// this gets multiply included - once to define the symbols, and again
// to instantiate. the order is significant - keep the first 4 eps
// sigma phi rho - at the start and the block symbols at the end.

/* Hypergraph arc special symbols.

   Meanings: epsilon and phi don't advance the input pointer when
   taking the arc (rho and sigma do).

   epsilon and sigma match any symbol; phi and rho are "else" - they
   match any symbol not explicitly named on another outgoing arc.
*/

/*
 * Hypergraph/Compose.hpp expects that eps, sigma, phi, rho are the
 * first lexical symbols (in that order). DO NOT CHANGE THIS.
 */


/// EPSILON::ID is 0, like in OpenFst.
/// The IDs for sigma, phi, and rho are different from the
/// corresponding IDs in OpenFst, where they are negative numbers.
SPECIAL_SYMBOL(EPSILON, <eps>, kSpecialTerminal)
SPECIAL_SYMBOL(SIGMA, <sigma>, kSpecialTerminal)
SPECIAL_SYMBOL(PHI, <phi>, kSpecialTerminal)
SPECIAL_SYMBOL(RHO, <rho>, kSpecialTerminal)

// TODO: these can return to <s> and </s> for greater
// inter-operability with third party software once
// vocabulary bugs that prevent those tokens in data
// from being treated normally are resolved
// see http://jira:8080/jira/browse/CM-230









SPECIAL_SYMBOL(SEG_START, <xmt-segment>, kSpecialTerminal)
SPECIAL_SYMBOL(SEG_END, </xmt-segment>, kSpecialTerminal)

SPECIAL_SYMBOL(UNK, <unk>, kSpecialTerminal)

// Used by tokenizers to mark tokens.
// Example: <tok> 't' 'e' 's' 't' </tok>
SPECIAL_SYMBOL(TOK_START, <tok>, kSpecialTerminal)
SPECIAL_SYMBOL(TOK_END, </tok>, kSpecialTerminal)

// Used by RegexTokenizer to protect expressions from being split into
// tokens. Example: <tok-protect> 'e' '.' 'g' '.' </tok-protect>
SPECIAL_SYMBOL(TOK_PROTECT_START, <tok-protect>, kSpecialTerminal)
SPECIAL_SYMBOL(TOK_PROTECT_END, </tok-protect>, kSpecialTerminal)

// SPECIAL_SYMBOL(GLUE, <glue>, kSpecialTerminal)
SPECIAL_SYMBOL(GLUE, __LW_AT__, kSpecialTerminal)

// Used by TrieGrammar and Syntax decoder to separate source formula and preconditions


// Used by syntax decoder to denote the start of the foreign sentence
SPECIAL_SYMBOL(FS, <foreign-sentence>, kSpecialTerminal)

// Used by abortSegment to return the output of the canceled segment
SPECIAL_SYMBOL(ABORTSEGMENT, <abort-segment>, kSpecialTerminal)

//////////////////////////////

//////////////////////////////

// These are used by the decoders: They do not move any words or
// phrases into the block or out of the block.

// Each block in a sentence has an ID. Internally, the different
// <xmt-block> symbols in a sentence are mapped to different symbol
// IDs: BLOCK_START::ID+0, BLOCK_START::ID+1, BLOCK_START::ID+2, ...

// The main reason for this is that, by default, Hypergraphs have
// canonical labels, and all label states that start an <xmt_block>
// would be indistinguishible if they used the same symbol ID.

SPECIAL_SYMBOL(BLOCK_END, </xmt-block>, kSpecialTerminal)
SPECIAL_SYMBOL(BLOCK_START, <xmt-block>, kSpecialTerminal)






// end. Additional special symbols must be defined BEFORE the block
// symbols.
