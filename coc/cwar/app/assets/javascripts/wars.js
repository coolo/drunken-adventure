function setupWarriors() {
    $('#warriors').multiSelect(
	{ keepOrder: true ,
	  afterSelect: function(value){
	      $('#warriors option[value="'+value+'"]').remove();
	      $('#warriors').append($('<option></option>').attr('value',value).attr('selected', 'selected'));
	  }
	}
    );
}
