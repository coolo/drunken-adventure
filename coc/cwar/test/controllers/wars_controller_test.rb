require 'test_helper'

class WarsControllerTest < ActionController::TestCase
  setup do
    @war = wars(:one)
  end

  test "should get index" do
    get :index
    assert_response :success
    assert_not_nil assigns(:wars)
  end

  test "should get new" do
    get :new
    assert_response :success
  end

  test "should create war" do
    assert_difference('War.count') do
      post :create, war: {  }
    end

    assert_redirected_to war_path(assigns(:war))
  end

  test "should show war" do
    get :show, id: @war
    assert_response :success
  end

  test "should get edit" do
    get :edit, id: @war
    assert_response :success
  end

  test "should update war" do
    patch :update, id: @war, war: {  }
    assert_redirected_to war_path(assigns(:war))
  end

  test "should destroy war" do
    assert_difference('War.count', -1) do
      delete :destroy, id: @war
    end

    assert_redirected_to wars_path
  end
end
