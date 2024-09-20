extends AudioStreamPlayer
class_name FunAudioStreamPlayer

@export_range(0, 1.0) var pitch_variation := 0.25

# override warning but watever cause it works
func play(time: float = 0.0):
	pitch_scale = 1.0 + randf_range(-pitch_variation, pitch_variation)
	super(time)
