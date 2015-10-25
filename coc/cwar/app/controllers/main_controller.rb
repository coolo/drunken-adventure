class MainController < ApplicationController

  def index
    if session[:current_user_id]
      redirect_to :current
    else
      @user = User.new
    end
  end

  def set_user
    @user = User.find(params[:user][:id])
    session[:current_user_id] = @user.id
    redirect_to :current
  end

  def war
    if !session[:current_user_id]
       redirect_to :index and return
    end
    @user = User.find(session[:current_user_id])
    @war = War.last
    @estimates = Hash.new
    @war.warriors.each do |w|
      @estimates[w] = { 0 => [], 1 => [], 2 => [], 3 => [] }
      w.estimates.where(user: @user).each do |e|
        @estimates[w][e.stars].append(e.base)
      end
    end
  end

  def fillin(estimates, value, range)
    return false if range.blank?
    range.split(',').each do |r|
      if r.end_with? '-'
        r = r[0, r.length-1].to_i
        while r <= estimates.size
          estimates[r] = value
          r = r +1
        end
      else
        estimates[r.to_i] = value
      end
    end
    return true
  end
  
  def estimate
    @war = War.find(params[:war_id])
    @user = User.find(session[:current_user_id])
    @war.warriors.each do |w|
      estimates = Hash.new
      @war.warriors.size.times { |i| estimates[i+1] = 0 }
      seenone = fillin(estimates, 2, params[:twos][w.id.to_s])
      seenone = fillin(estimates, 3, params[:threes][w.id.to_s]) || seenone
      w.transaction do
        w.estimates.where(user: @user).delete_all
        if seenone
          estimates.each_pair do |base,value|
            w.estimates.build user: @user, base: base, stars: value
          end
          w.save
        end
      end
    end
    redirect_to :current
  end
  
end
