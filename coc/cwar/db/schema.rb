# encoding: UTF-8
# This file is auto-generated from the current state of the database. Instead
# of editing this file, please use the migrations feature of Active Record to
# incrementally modify your database, and then regenerate this schema definition.
#
# Note that this schema.rb definition is the authoritative source for your
# database schema. If you need to create the application database on another
# system, you should be using db:schema:load, not running all the migrations
# from scratch. The latter is a flawed and unsustainable approach (the more migrations
# you'll amass, the slower it'll run and the greater likelihood for issues).
#
# It's strongly recommended that you check this file into your version control system.

ActiveRecord::Schema.define(version: 20151026123725) do

  create_table "estimates", force: :cascade do |t|
    t.integer  "user_id"
    t.integer  "warrior_id"
    t.integer  "base"
    t.integer  "stars"
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
  end

  create_table "users", force: :cascade do |t|
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
    t.string   "name"
  end

  create_table "users_wars", id: false, force: :cascade do |t|
    t.integer "war_id",  null: false
    t.integer "user_id", null: false
  end

  add_index "users_wars", ["user_id", "war_id"], name: "index_users_wars_on_user_id_and_war_id", unique: true

  create_table "warriors", force: :cascade do |t|
    t.integer  "war_id"
    t.integer  "user_id"
    t.integer  "order"
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
  end

  create_table "wars", force: :cascade do |t|
    t.string   "title"
    t.integer  "warriors"
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
  end

end
