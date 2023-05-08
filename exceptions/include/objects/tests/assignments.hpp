TEST(Assignments, Simple) {
    ResetStats();

    const std::string value_1 = "plain";
    const std::string value_2 = "another plain";

    AutoObject<Plain> sample_1{value_1};
    AutoObject<Plain> sample_2{value_2};
    AutoObject<Plain> sample_3;
    Plain sample_4{value_1};
    Plain sample_5;

    ASSERT_EQ(sample_1 DOT index, value_1.size());
    ASSERT_EQ(sample_2 DOT index, value_2.size());
    ASSERT_EQ(sample_3 DOT index, 0);
    ASSERT_EQ(sample_4.index, value_1.size());
    ASSERT_EQ(sample_5.index, 0);

    // AutoObject = AutoObject

    sample_1 = sample_2;
    ASSERT_EQ(sample_2 DOT index, value_2.size());
    ASSERT_EQ(sample_1 DOT index, value_2.size());

    sample_3 = std::move(sample_2);
    ASSERT_EQ(sample_3 DOT index, value_2.size());

    // AutoObject = Plain

    sample_2 = sample_4;
    ASSERT_EQ(sample_4.index, value_1.size());
    ASSERT_EQ(sample_2 DOT index, value_1.size());

    sample_1 = std::move(sample_4);
    ASSERT_EQ(sample_1 DOT index, value_1.size());

    // Plain = AutoObject

    sample_4 = sample_2;
    ASSERT_EQ(sample_2 DOT index, value_1.size());
    ASSERT_EQ(sample_4.index, value_1.size());

#ifdef FINAL
    sample_5 = std::move(sample_3).Move();
#else
    sample_5 = std::move(sample_3);
#endif
    ASSERT_EQ(sample_5.index, value_2.size());

    // With const AutoObject (std::move does not do anything with const values)

    const AutoObject<Plain> sample_const{value_1};
    sample_1 = sample_const;
    sample_4 = sample_const;
    ASSERT_EQ(sample_const DOT index, value_1.size());
    ASSERT_EQ(sample_1 DOT index, value_1.size());
    ASSERT_EQ(sample_4.index, value_1.size());

    // clang-format off
    const Stats expected{
        .ctor{
            .default_        = 2,  // sample_3, sample_5
            .single_argument = 4,  // sample_1, sample_2, sample_4, sample_const
        },
        .assign{
            .copy = 5,
            .move = 3,
        },
    };
    // clang-format on
    ASSERT_EQ(Plain::stats, expected);

    // static checks

    static_assert(std::is_copy_assignable_v<AutoObject<Plain>>);
    static_assert(std::is_copy_assignable_v<AutoObject<Explicit>>);
    static_assert(std::is_copy_assignable_v<AutoObject<Noexcept>>);

    static_assert(std::is_assignable_v<AutoObject<Plain>, const Plain&>);
    static_assert(std::is_assignable_v<AutoObject<Explicit>, const Explicit&>);
    static_assert(std::is_assignable_v<AutoObject<Noexcept>, const Noexcept&>);

    static_assert(std::is_move_assignable_v<AutoObject<Plain>>);
    static_assert(std::is_move_assignable_v<AutoObject<Explicit>>);
    static_assert(std::is_move_assignable_v<AutoObject<Noexcept>>);

    static_assert(std::is_assignable_v<AutoObject<Plain>, Plain>);
    static_assert(std::is_assignable_v<AutoObject<Explicit>, Explicit>);
    static_assert(std::is_assignable_v<AutoObject<Noexcept>, Noexcept>);

    // noexcept checks

    static_assert(not std::is_nothrow_copy_assignable_v<AutoObject<Plain>>);
    static_assert(not std::is_nothrow_copy_assignable_v<AutoObject<Explicit>>);
    static_assert(std::is_nothrow_copy_assignable_v<AutoObject<Noexcept>>);

    static_assert(not std::is_nothrow_assignable_v<AutoObject<Plain>,  //
                                                   const Plain&>);
    static_assert(not std::is_nothrow_assignable_v<AutoObject<Explicit>,
                                                   const Explicit&>);
    static_assert(std::is_nothrow_assignable_v<AutoObject<Noexcept>,  //
                                               const Noexcept&>);

    static_assert(not std::is_nothrow_move_assignable_v<AutoObject<Plain>>);
    static_assert(not std::is_nothrow_move_assignable_v<AutoObject<Explicit>>);
    static_assert(std::is_nothrow_move_assignable_v<AutoObject<Noexcept>>);

    static_assert(not std::is_nothrow_assignable_v<AutoObject<Plain>,  //
                                                   Plain>);
    static_assert(not std::is_nothrow_assignable_v<AutoObject<Explicit>,  //
                                                   Explicit>);
    static_assert(std::is_nothrow_assignable_v<AutoObject<Noexcept>,  //
                                               Noexcept>);
}
