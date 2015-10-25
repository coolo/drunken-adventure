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
    @plan = [ '2_1', '1_2', '3_3', '5_4', '4_5', '6_6', '4_7', '7_8', '9_9', '10_10', '8_11', '16_12', '11_13','12_14', '15_15', '18_20', '19_19', '19_18', '17_17', '13_16']
  end
  
  private
    # Use callbacks to share common setup or constraints between actions.
    def set_war
      @war = War.find(params[:id])
    end

    # Never trust parameters from the scary internet, only allow the white list through.
    def war_params
      params[:war].permit(:title, :order => [])
    end
end
