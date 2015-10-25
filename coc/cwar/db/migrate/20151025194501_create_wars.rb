class CreateWars < ActiveRecord::Migration
  def change
    create_table :wars do |t|
      t.string :title
      t.integer :warriors

      t.timestamps null: false
    end
  end
end
