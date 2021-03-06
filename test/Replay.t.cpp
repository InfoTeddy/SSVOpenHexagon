// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/Replay.hpp"

#include "TestUtils.hpp"

#include <random>

static void test_replay_data_basic()
{
    hg::replay_data rd;
    TEST_ASSERT_EQ(rd.size(), 0);

    rd.record_input(false, false, false, false);
    TEST_ASSERT_EQ(rd.size(), 1);
    TEST_ASSERT_EQ(rd.at(0), hg::input_bitset{});

    rd.record_input(true, false, true, false);
    TEST_ASSERT_EQ(rd.size(), 2);
    TEST_ASSERT_EQ(rd.at(0), hg::input_bitset{});
    TEST_ASSERT_EQ(rd.at(1), hg::input_bitset{"0101"});
}

static void test_replay_data_serialization_to_buffer()
{
    hg::replay_data rd;
    TEST_ASSERT_EQ(rd.size(), 0);

    rd.record_input(false, false, false, false);
    rd.record_input(false, true, false, false);
    rd.record_input(true, false, true, false);
    rd.record_input(false, false, false, false);
    rd.record_input(false, true, false, false);
    rd.record_input(true, false, false, true);

    TEST_ASSERT_EQ(rd.size(), 6);
    TEST_ASSERT_EQ(rd.at(0), hg::input_bitset{"0000"});
    TEST_ASSERT_EQ(rd.at(1), hg::input_bitset{"0010"});
    TEST_ASSERT_EQ(rd.at(2), hg::input_bitset{"0101"});
    TEST_ASSERT_EQ(rd.at(3), hg::input_bitset{"0000"});
    TEST_ASSERT_EQ(rd.at(4), hg::input_bitset{"0010"});
    TEST_ASSERT_EQ(rd.at(5), hg::input_bitset{"1001"});

    constexpr std::size_t buf_size{1024};
    std::byte buf[buf_size];

    TEST_ASSERT_NS(rd.serialize(buf, buf_size));

    hg::replay_data rd_out;
    TEST_ASSERT_NS(rd_out.deserialize(buf, buf_size));

    TEST_ASSERT_NS_EQ(rd_out, rd);
}

static void test_replay_data_serialization_to_buffer_too_small()
{
    hg::replay_data rd;
    TEST_ASSERT_EQ(rd.size(), 0);

    rd.record_input(false, false, false, false);
    rd.record_input(false, true, false, false);
    rd.record_input(true, false, true, false);
    rd.record_input(false, false, false, false);
    rd.record_input(false, true, false, false);
    rd.record_input(true, false, false, true);

    TEST_ASSERT_EQ(rd.size(), 6);
    TEST_ASSERT_EQ(rd.at(0), hg::input_bitset{"0000"});
    TEST_ASSERT_EQ(rd.at(1), hg::input_bitset{"0010"});
    TEST_ASSERT_EQ(rd.at(2), hg::input_bitset{"0101"});
    TEST_ASSERT_EQ(rd.at(3), hg::input_bitset{"0000"});
    TEST_ASSERT_EQ(rd.at(4), hg::input_bitset{"0010"});
    TEST_ASSERT_EQ(rd.at(5), hg::input_bitset{"1001"});

    constexpr std::size_t buf_size{10};
    std::byte buf[buf_size];

    TEST_ASSERT_NS(!rd.serialize(buf, buf_size));
}

static void test_replay_player_basic()
{
    hg::replay_data rd;

    rd.record_input(false, false, false, false);
    rd.record_input(false, true, false, false);
    rd.record_input(true, false, true, false);
    rd.record_input(false, false, false, false);
    rd.record_input(false, true, false, false);
    rd.record_input(true, false, false, true);

    hg::replay_player rp{rd};

    TEST_ASSERT_EQ(rp.get_current_and_move_forward(), hg::input_bitset{"0000"});
    TEST_ASSERT(!rp.done());

    TEST_ASSERT_EQ(rp.get_current_and_move_forward(), hg::input_bitset{"0010"});
    TEST_ASSERT(!rp.done());

    TEST_ASSERT_EQ(rp.get_current_and_move_forward(), hg::input_bitset{"0101"});
    TEST_ASSERT(!rp.done());

    TEST_ASSERT_EQ(rp.get_current_and_move_forward(), hg::input_bitset{"0000"});
    TEST_ASSERT(!rp.done());

    TEST_ASSERT_EQ(rp.get_current_and_move_forward(), hg::input_bitset{"0010"});
    TEST_ASSERT(!rp.done());

    TEST_ASSERT_EQ(rp.get_current_and_move_forward(), hg::input_bitset{"1001"});
    TEST_ASSERT(rp.done());

    TEST_ASSERT_EQ(rp.get_current_and_move_forward(), hg::input_bitset{});
    TEST_ASSERT(rp.done());

    TEST_ASSERT_EQ(rp.get_current_and_move_forward(), hg::input_bitset{});
    TEST_ASSERT(rp.done());

    TEST_ASSERT_EQ(rp.get_current_and_move_forward(), hg::input_bitset{});
    TEST_ASSERT(rp.done());

    TEST_ASSERT_EQ(rp.get_current_and_move_forward(), hg::input_bitset{});
    TEST_ASSERT(rp.done());
}

static void test_replay_file_serialization_to_buffer()
{
    hg::replay_data rd;

    rd.record_input(false, false, false, false);
    rd.record_input(false, true, false, false);
    rd.record_input(true, false, true, false);
    rd.record_input(false, false, false, false);
    rd.record_input(false, true, false, false);
    rd.record_input(true, false, false, true);

    hg::replay_file rf{
        //
        ._version{59832},
        ._player_name{"hello world"},
        ._seed{12345},
        ._data{rd},
        ._pack_id{"totally real pack id"},
        ._level_id{"legit level id"},
        ._first_play{false},
        ._difficulty_mult{2.5f},
        ._played_score{100.f}
        //
    };

    constexpr std::size_t buf_size{2048};
    std::byte buf[buf_size];

    TEST_ASSERT_NS(rf.serialize(buf, buf_size));

    hg::replay_file rf_out;
    TEST_ASSERT_NS(rf_out.deserialize(buf, buf_size));

    TEST_ASSERT_NS_EQ(rf_out, rf);
}

static void test_replay_file_serialization_to_file()
{
    hg::replay_data rd;

    rd.record_input(false, false, false, false);
    rd.record_input(false, true, false, false);
    rd.record_input(true, false, true, false);
    rd.record_input(false, false, false, false);
    rd.record_input(false, true, false, false);
    rd.record_input(true, false, false, true);

    hg::replay_file rf{
        //
        ._version{59832},
        ._player_name{"hello world"},
        ._seed{1234512345},
        ._data{rd},
        ._pack_id{"totally real pack id"},
        ._level_id{"legit level id"},
        ._first_play{true},
        ._difficulty_mult{2.5f},
        ._played_score{100.f}
        //
    };

    TEST_ASSERT(rf.serialize_to_file("test.ohr"));

    hg::replay_file rf_out;
    TEST_ASSERT(rf_out.deserialize_from_file("test.ohr"));

    TEST_ASSERT_NS_EQ(rf_out, rf);
}

[[nodiscard]] static auto& getRng()
{
    static std::random_device rd;
    static std::mt19937 rng(rd());

    return rng;
}

[[nodiscard]] static float getRndFloat(float min, float max)
{
    return std::uniform_real_distribution<float>{min, max}(getRng());
}

template <typename T>
[[nodiscard]] static T getRndInt(T min, T max)
{
    return std::uniform_int_distribution<T>{min, max}(getRng());
}

[[nodiscard]] static bool getRndBool()
{
    return getRndInt<int>(0, 10) > 5;
}

static void test_replay_file_serialization_to_file_randomized()
{
    hg::replay_data rd;

    const int nInputs = getRndInt<int>(0, 4096);
    for(int i = 0; i < nInputs; ++i)
    {
        rd.record_input(getRndBool(), getRndBool(), getRndBool(), getRndBool());
    }

    hg::replay_file rf{
        //
        ._version{getRndInt<std::uint32_t>(0, 1000000)},
        ._player_name{"hello world"},
        ._seed{getRndInt<hg::replay_file::seed_type>(0, 1000000)},
        ._data{rd},
        ._pack_id{"totally real pack id"},
        ._level_id{"legit level id"},
        ._difficulty_mult{getRndFloat(0.0f, 100000.0f)},
        ._played_score{getRndFloat(0.0f, 100000.0f)}
        //
    };

    TEST_ASSERT(rf.serialize_to_file("test.ohr"));

    hg::replay_file rf_out;
    TEST_ASSERT(rf_out.deserialize_from_file("test.ohr"));

    TEST_ASSERT_NS_EQ(rf_out, rf);
}

int main()
{
    test_replay_data_basic();
    test_replay_data_serialization_to_buffer();
    test_replay_data_serialization_to_buffer_too_small();

    test_replay_player_basic();

    test_replay_file_serialization_to_buffer();
    test_replay_file_serialization_to_file();

    for(int i = 0; i < 256; ++i)
    {
        test_replay_file_serialization_to_file_randomized();
    }
}
