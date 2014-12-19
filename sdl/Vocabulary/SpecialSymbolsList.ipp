































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



SPECIAL_SYMBOL(TOK_START, <tok>, kSpecialTerminal)
SPECIAL_SYMBOL(TOK_END, </tok>, kSpecialTerminal)



SPECIAL_SYMBOL(TOK_PROTECT_START, <tok-protect>, kSpecialTerminal)
SPECIAL_SYMBOL(TOK_PROTECT_END, </tok-protect>, kSpecialTerminal)








SPECIAL_SYMBOL(FS, <foreign-sentence>, kSpecialTerminal)





























