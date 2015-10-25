class War < ActiveRecord::Base
  has_many :warriors, dependent: :delete_all
  has_many :estimates

  def count
    @count || @count = warriors.count
  end
  
  def set_order(order)
    self.warriors = []
    index = 1
    order.each do |id|
      self.warriors.build(user: User.find(id), order: index)
      index = index + 1
    end
  end
  
  def update(params)
    set_order(params.delete :order)
    super
  end

end
