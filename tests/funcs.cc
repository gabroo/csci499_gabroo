#include "gtest/gtest.h"
#include "kvstore/db.h"
#include "kvstore/store.h"
#include "warble/functions.h"

class FakeDB : public Database {
 public:
  bool put(std::string key, std::string val) { return store_.put(key, val); }
  std::optional<std::vector<std::string>> get(std::string key) {
    return store_.get(key);
  }
  bool remove(std::string key) { return store_.remove(key); }

 private:
  KVStore store_;
};

class FuncsTest : public ::testing::Test {
 protected:
  FakeDB db;
  void SetUp() override {
    db = FakeDB();
    db.put("_users_", "gabroo");
    db.put("_users_", "gabru");
    db.put("_users_", "gabbru");
    db.put("_users_", "gabbroo");
  }
};

TEST_F(FuncsTest, RegisterUser) {
  Any req, rep;
  RegisterUserRequest ru_req;
  ru_req.set_username("test_user");
  req.PackFrom(ru_req);
  EXPECT_TRUE(RegisterUser(&db, req, &rep));
  EXPECT_FALSE(RegisterUser(&db, req, &rep));
}

TEST_F(FuncsTest, Warble) {
  Any req, rep;
  WarbleRequest w_req;
  w_req.set_username("gabroo");
  w_req.set_text("hello world");
  req.PackFrom(w_req);
  EXPECT_TRUE(Warble(&db, req, &rep));
  w_req.set_username("not_a_user_that_exists");
  req.PackFrom(w_req);
  EXPECT_FALSE(Follow(&db, req, &rep));
}

TEST_F(FuncsTest, Follow) {
  Any req, rep;
  FollowRequest f_req;
  f_req.set_username("gabroo");
  f_req.set_to_follow("gabru");
  req.PackFrom(f_req);
  EXPECT_TRUE(Follow(&db, req, &rep));
  f_req.set_to_follow("not_a_user_that_exists");
  req.PackFrom(f_req);
  EXPECT_FALSE(Follow(&db, req, &rep));
}

TEST_F(FuncsTest, Read) {
  Any req, rep;
  WarbleRequest w_req;
  w_req.set_username("gabroo");
  w_req.set_text("hello world");
  req.PackFrom(w_req);
  EXPECT_TRUE(Warble(&db, req, &rep));
  w_req.set_username("gabroo");
  w_req.set_text("hello world");
  w_req.set_parent_id("gabroo0");
  req.PackFrom(w_req);
  EXPECT_TRUE(Warble(&db, req, &rep));
  ReadRequest r_req;
  r_req.set_warble_id("gabroo0");
  req.PackFrom(r_req);
  EXPECT_TRUE(Read(&db, req, &rep));
  ReadReply r_rep;
  rep.UnpackTo(&r_rep);
  EXPECT_EQ(r_rep.warbles_size(), 2);
}

TEST_F(FuncsTest, Profile) {
  Any req, rep;
  FollowRequest f_req;
  f_req.set_username("gabroo");
  f_req.set_to_follow("gabru");
  req.PackFrom(f_req);
  EXPECT_TRUE(Follow(&db, req, &rep));
  f_req.set_to_follow("gabbroo");
  req.PackFrom(f_req);
  EXPECT_TRUE(Follow(&db, req, &rep));
  f_req.set_to_follow("gabbru");
  req.PackFrom(f_req);
  EXPECT_TRUE(Follow(&db, req, &rep));
  f_req.set_username("gabru");
  f_req.set_to_follow("gabroo");
  req.PackFrom(f_req);
  EXPECT_TRUE(Follow(&db, req, &rep));
  ProfileRequest p_req;
  p_req.set_username("gabroo");
  req.PackFrom(p_req);
  EXPECT_TRUE(Profile(&db, req, &rep));
  ProfileReply p_rep;
  rep.UnpackTo(&p_rep);
  EXPECT_EQ(p_rep.following_size(), 3);
  EXPECT_EQ(p_rep.followers_size(), 1);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}