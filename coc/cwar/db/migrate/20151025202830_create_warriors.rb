class CreateWarriors < ActiveRecord::Migration
  def change
    create_table :warriors do |t|
      t.belongs_to :war
      t.belongs_to :user
      t.integer :order
      t.timestamps null: false
    end
  end
end
