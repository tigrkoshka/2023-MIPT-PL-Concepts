TEST(Methods, Simple) {
    const std::string value = "plain";

    // both const and non-const methods are available for non-const instances

    AutoObject<Plain> sample = value;
    ASSERT_EQ(sample DOT Index(), value.size());
    sample DOT IncrementIndex();
    ASSERT_EQ(sample DOT Index(), value.size() + 1);

    // only const methods are available on const instances

    const AutoObject<Plain> const_sample = value;
    ASSERT_EQ(const_sample DOT Index(), value.size());
    // const_sample.IncrementIndex();

    // static checks

    // TODO
//    static_assert(CanIndex<AutoObject<Plain>>);
//    static_assert(CanIndex<const AutoObject<Plain>>);
//
//    static_assert(CanIncrementIndex<AutoObject<Plain>>);
//    static_assert(not CanIncrementIndex<const AutoObject<Plain>>);
}
