class User < ActiveRecord::Base

  has_many :estimates
  @@users = Hash.new
  
  def self.name_for(id)
    if @@users.empty?
      User.all.each do |u|
        @@users[u.id] = u.name
      end
    end
    @@users[id]
  end
  
  def warrior_in?(war)
    war.warriors.each do |w|
      if w.user == self
        return true
      end
    end
    false
  end
end
