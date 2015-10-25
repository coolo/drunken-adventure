class User < ActiveRecord::Base

  has_many :estimates

  def warrior_in?(war)
    war.warriors.each do |w|
      if w.user == self
        return true
      end
    end
    false
  end
end
