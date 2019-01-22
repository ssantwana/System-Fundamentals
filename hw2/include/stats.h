





Stats *statistics(Course *c);
Stats *buildstats(Course *c);
void do_links(Course *c, Stats *s);
void do_freqs(Course *c);
Freqs *count_score(Score *scp, Freqs *afp);
void do_quantiles(Stats *s);
void do_sums(Course *c);
void do_moments(Stats *s);
double stddev(int n, double sum, double sumsq);
