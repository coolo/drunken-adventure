class CreateEstimates < ActiveRecord::Migration
  def change
    create_table :estimates do |t|

      t.belongs_to :user
      t.belongs_to :warrior
      t.integer :base
      t.integer :stars

      t.timestamps null: false
    end
  end
end
