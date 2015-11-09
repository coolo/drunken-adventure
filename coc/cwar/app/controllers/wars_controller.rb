class WarsController < ApplicationController
  before_action :set_war, only: [:show, :edit, :update, :destroy, :plan]

  # GET /wars
  # GET /wars.json
  def index
    @wars = War.all
  end

  # GET /wars/1
  # GET /wars/1.json
  def show
  end

  # GET /wars/new
  def new
    @war = War.new
  end

  # GET /wars/1/edit
  def edit
  end

  # POST /wars
  # POST /wars.json
  def create
    filtered_params = war_params
    order = filtered_params.delete :order
    @war = War.new(filtered_params)
    @war.set_order(order)
    
    respond_to do |format|
      if @war.save
        format.html { redirect_to @war, notice: 'War was successfully created.' }
        format.json { render :show, status: :created, location: @war }
      else
        format.html { render :new }
        format.json { render json: @war.errors, status: :unprocessable_entity }
      end
    end
  end

  # PATCH/PUT /wars/1
  # PATCH/PUT /wars/1.json
  def update
    respond_to do |format|
      if @war.update(war_params)
        format.html { redirect_to @war, notice: 'War was successfully updated.' }
        format.json { render :show, status: :ok, location: @war }
      else
        format.html { render :edit }
        format.json { render json: @war.errors, status: :unprocessable_entity }
      end
    end
  end

  # DELETE /wars/1
  # DELETE /wars/1.json
  def destroy
    @war.destroy
    respond_to do |format|
      format.html { redirect_to wars_url, notice: 'War was successfully destroyed.' }
      format.json { head :no_content }
    end
  end

  def plan
    @warriors = @war.warriors
    @warriors.each_with_index do |w,i|
      w.index = i + 1
    end
    sorted = @warriors.sort { |a,b| b.index_avg <=> a.index_avg }
    @plan = []
    @war.count.times do |i|
      @plan.append("#{sorted[i].index}_#{i+1}")
    end
  end
  
  private
    # Use callbacks to share common setup or constraints between actions.
    def set_war
      @war = War.includes(:warriors, warriors: :estimates).find(params[:id])
    end

    # Never trust parameters from the scary internet, only allow the white list through.
    def war_params
      params[:war].permit(:title, :order => [])
    end
end
