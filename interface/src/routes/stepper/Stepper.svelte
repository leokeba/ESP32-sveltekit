<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { user } from '$lib/stores/user';
	import { page } from '$app/stores';
	import { notifications } from '$lib/components/toasts/notifications';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import Stepper from '~icons/tabler/rotate';
	import Info from '~icons/tabler/info-circle';
	import Save from '~icons/tabler/device-floppy';
	import Stop from '~icons/tabler/hand-stop';

	type StepperControl = {
		isEnabled: boolean;
		direction: boolean;
		speed: number;
		acceleration: number;
		current: number;
	};

	let stepperControl: StepperControl = { isEnabled: false, direction: false, speed: 128, acceleration: 30, current: 800 };

	let lightOn = false;
	let lightBrightness = 128;

	const ws_token = $page.data.features.security ? '?access_token=' + $user.bearer_token : '';

	const stepperControlSocket = new WebSocket('ws://' + $page.url.host + '/ws/stepperControl' + ws_token);

	stepperControlSocket.onopen = (event) => {
		stepperControlSocket.send('Hello');
	};

	stepperControlSocket.addEventListener('close', (event) => {
		const closeCode = event.code;
		const closeReason = event.reason;
		console.log('WebSocket closed with code:', closeCode);
		console.log('Close reason:', closeReason);
		notifications.error('Websocket disconnected', 5000);
	});

	stepperControlSocket.onmessage = (event) => {
		const message = JSON.parse(event.data);
		if (message.type != 'id') {
			stepperControl = message;
		}
	};

	onDestroy(() => stepperControlSocket.close());

</script>


<style>
	.grid-form {
	  grid-template-columns: [labels] auto [controls] 1fr;
	}
	/* .grid-form {
	  display: grid;
	  grid-template-columns: [labels] auto [controls] 1fr;
	  grid-auto-flow: row;
	}
	.grid-form label  {
	  grid-column: labels;
	  grid-row: auto;
	}
	.grid-form input {
	  grid-column: controls;
	  grid-row: auto;
	} */
</style>

<SettingsCard collapsible={true}>
	<Stepper slot="icon" class="flex-shrink-0 mr-2 h-6 w-6 self-end" />
	<span slot="title">Stepper</span>
	<div class="w-full">
		<div class="w-full grid grid-flow-row grid-form items-center">
			<label class="label cursor-pointer" for="enable">
				<span class="">Enable</span>
			</label>
			<input
				type="checkbox"
				class="toggle toggle-primary"
				id="enable"
				bind:checked={stepperControl.isEnabled}
				on:change={() => {
					stepperControlSocket.send(JSON.stringify(stepperControl));
				}}
			/>
			<label class="label cursor-pointer" for="direction">
				<span class="">Direction</span>
			</label>
			<input
				type="checkbox"
				class="toggle toggle-primary"
				id="direction"
				bind:checked={stepperControl.direction}
				on:change={() => {
					stepperControlSocket.send(JSON.stringify(stepperControl));
				}}
			/>
			<label class="label cursor-pointer" for="speed">
				<span class="mr-4">Speed </span>
			</label>
			<input 
				type="range"
				min="0" 
				max="400" 
				class="range range-primary"
				id="speed"
				bind:value={stepperControl.speed}
				on:change={() => {
					stepperControlSocket.send(JSON.stringify(stepperControl));
				}}
			/>
			<label class="label cursor-pointer" for="acceleration">
				<span class="mr-4">Acceleration </span>
			</label>
			<input 
				type="range"
				min="0" 
				max="200" 
				class="range range-primary"
				id="acceleration"
				bind:value={stepperControl.acceleration}
				on:change={() => {
					stepperControlSocket.send(JSON.stringify(stepperControl));
				}}
			/>
			<label class="label cursor-pointer" for="current">
				<span class="mr-4">Current </span>
			</label>
			<input 
				type="range"
				min="0" 
				max="4000" 
				class="range range-primary"
				id="current"
				bind:value={stepperControl.current}
				on:change={() => {
					stepperControlSocket.send(JSON.stringify(stepperControl));
				}}
			/>
		</div>
		<div class="flex flex-row flex-wrap justify-between gap-x-2">
			<div class="flex-grow"></div>
			<div>
				<div>
					<button class="btn btn-primary inline-flex items-center" on:click={() => {stepperControl.speed=0; stepperControlSocket.send(JSON.stringify(stepperControl));}}
						><Stop class="mr-2 h-5 w-5" /><span>Stop</span></button
					>
				</div>
			</div>
		</div>
	</div>
</SettingsCard>
